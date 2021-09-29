#pragma pack_matrix(row_major)

#include "ShaderUtils.hlsl"

Texture2D GBufferA : register(t0); // normal
Texture2D GBufferB : register(t1); // MetallicSpecularRoughness
Texture2D GBufferC : register(t2); // AlbedoAO
Texture2D SceneDepthZ : register(t3); // Depth
Texture2D<float2> HiZBuffer : register(t4); // Hi-Z Depth
Texture2D HistorySceneColor : register(t5); // history scene buffer
Texture2D VelocityBuffer : register(t6); // velocity buffer
TextureCube CubeMap : register(t7); // environment map

SamplerState LinearSampler : register(s0);
SamplerState PointSampler : register(s1);

cbuffer PSContant : register(b0)
{
    float4x4 ViewProj;
    float4x4 InvViewProj;
    float4x4 ClipToPreClipNoAA;
    float4 RootSizeMipCount;
    float4 HZBUvFactorAndInvFactor;
    float3 CameraPos;
    float Thickness;
    float WorldThickness;
    float CompareTolerance;
    float UseHiZ;
    float UseMinMaxZ;
    int NumRays;
    int FrameIndexMod8;
};

static const int MaxStep = 256;

static const float HIZ_START_LEVEL = 0.0f;
static const float HIZ_STOP_LEVEL = 0.0f;
static const float MAX_ITERATIONS = 256.0;

float3 ProjectWorldPos(float3 WorldPos)
{
    float4 ClipPos = mul(float4(WorldPos, 1.0), ViewProj);
    float3 Projected = ClipPos.xyz / ClipPos.w;
    Projected.xy = Projected.xy * float2(0.5, -0.5) + 0.5; //[-1,1] - > [0,1]
    return Projected;
}

float3 UnprojectScreen(float3 ScreenPoint)
{
    ScreenPoint.xy = float2(2.0, -2.0) * ScreenPoint.xy + float2(-1.0, 1.0); //[0,1] -> [-1,1]
    float4 WorldPos = mul(float4(ScreenPoint, 1.0), InvViewProj);
    return WorldPos.xyz / WorldPos.w;
}

float3 ApplyHZBUvFactor(float3 ScreenPos)
{
    return float3(ScreenPos.xy * HZBUvFactorAndInvFactor.xy * 0.5, ScreenPos.z);
}

float3 IntersectDepthPlane(float3 RayOrigin, float3 RayDir, float t)
{
    return RayOrigin + RayDir * t;
}

float2 GetCellCount(float2 Size, float Level)
{
    return floor(Size / (Level > 0.0 ? exp2(Level) : 1.0));
}

float2 GetCell(float2 Ray, float2 CellCount)
{
    return floor(Ray * CellCount);
}

bool CrossedCellBoundary(float2 CellIdxA, float2 CellIdxB)
{
    return CellIdxA.x != CellIdxB.x || CellIdxA.y != CellIdxB.y;
}

float GetMinimunDepthPlane(float2 Ray,float Level)
{
    return HiZBuffer.SampleLevel(PointSampler, float2(Ray.x, Ray.y), Level).r;
}

float2 GetMinMaxDepthPlanes(float2 Ray, float Level)
{
    return HiZBuffer.SampleLevel(PointSampler, float2(Ray.x, Ray.y), Level).rg;
}

float3 IntersectCellBoundray(
    float3 RayOrigin,float3 RayDirection,
    float2 CellIndex,float2 CellCount,
    float2 CrossStep,float2 CrossOffset
)
{
    float2 Cell = CellIndex + CrossStep;
    Cell /= CellCount;
    Cell += CrossOffset;
    
    float2 delta = Cell - RayOrigin.xy;
    delta /= RayDirection.xy;
    float t = min(delta.x, delta.y);
    return IntersectDepthPlane(RayOrigin, RayDirection, t);
}

bool CastSimpleRay(float3 Start, float3 Direction, float ScreenDistance, out float3 OutHitUVz)
{
    float PerPixelThickness = ScreenDistance;
    float PerPixelCompareBias = 0.85 * PerPixelThickness;
    
    float2 TextureSize;
    SceneDepthZ.GetDimensions(TextureSize.x, TextureSize.y);
    int MaxLinerStep = max(TextureSize.x, TextureSize.y);
    
    Direction = normalize(Direction);
    float3 Step = Direction;
    float StepScale = abs(Direction.x) > abs(Direction.y) ? TextureSize.x : TextureSize.y;
    Step /= StepScale;
    
    float Depth;
    float3 Ray = Start;
    for (int i = 0; i < MaxLinerStep; ++i)
    {
        Ray += Step;
        if(Ray.z < 0 || Ray.z > 1)
            return false;
        Depth = SceneDepthZ.SampleLevel(PointSampler, Ray.xy, 0).x;
        if (Depth + PerPixelCompareBias < Ray.z && Ray.z < Depth + PerPixelThickness)
        {
            OutHitUVz = Ray;
            return true;
        }
    }

    return true;
}

bool WithinThickness(float3 Ray, float MinZ, float TheThickness)
{
    return Ray.z < MinZ + TheThickness;
}

bool CastHiZRay(float3 Start, float3 Direction, float ScreenDistance, out float3 OutHitUVz)
{
    float PerPixelThickness = ScreenDistance;
    float PerPixelCompareBias = 0.85 * PerPixelThickness;
    
    Direction = normalize(Direction);
    
    const float2 TextureSize = RootSizeMipCount.xy;
    const float HIZ_MAX_LEVEL = RootSizeMipCount.z - 1;
    float2 HIZ_CROSS_EPSILON = 0.05 / TextureSize; // 0.5 in original paper, smaller value generate better result
    
    float Level = HIZ_START_LEVEL;
    float Iteration = 0.f;
    
    float2 CrossStep = sign(Direction.xy);
    float2 CrossOffset = CrossStep * HIZ_CROSS_EPSILON;
	// for negative direction, the starting point is top-left corner, 'CrossOffset' is enough to step back one cell
    CrossStep = saturate(CrossStep);
    
    float3 Ray = Start;
    float3 D = Direction.xyz / Direction.z;
    float3 O = IntersectDepthPlane(Start, D, -Start.z);
    
    bool intersected = false;
    float2 RayCell = GetCell(Ray.xy, TextureSize);
    Ray = IntersectCellBoundray(O, D, RayCell, TextureSize, CrossStep, CrossOffset);
    while(Level >= HIZ_STOP_LEVEL && Iteration < MAX_ITERATIONS)
    {
        const float2 CellCount = GetCellCount(TextureSize, Level);
        const float2 OldCellIdx = GetCell(Ray.xy, CellCount);
        if(Ray.z > 1.0)
            return false;
        float2 MinMaxZ = GetMinMaxDepthPlanes(Ray.xy, Level);
        float t = UseMinMaxZ > 0.f ? clamp(Ray.z, MinMaxZ.x + PerPixelCompareBias, MinMaxZ.y + PerPixelCompareBias) : max(Ray.z, MinMaxZ.x + PerPixelCompareBias);
        float3 TempRay = IntersectDepthPlane(O, D, t);
        const float2 NewCellIdx = GetCell(TempRay.xy, CellCount);
        if (CrossedCellBoundary(OldCellIdx,NewCellIdx))
        {
            TempRay = IntersectCellBoundray(O, D, OldCellIdx, CellCount, CrossStep, CrossOffset);
            Level = min(HIZ_MAX_LEVEL, Level + 2);
        }
        else if (Level == HIZ_START_LEVEL && WithinThickness(TempRay, MinMaxZ.x, PerPixelThickness))
        {
            intersected = true;
        }
        Ray = TempRay;
        --Level;
        ++Iteration;
    }
    OutHitUVz = Ray;
    return intersected;
}

float ComputeHitVignetteFromScreenPos(float2 ScreenPos)
{
    float2 Vignette = saturate(abs(ScreenPos) * 5 - 4);

    return saturate(1.0 - dot(Vignette, Vignette));
}

void ReprojectHit(float3 HitUVz, out float2 OutPrevUV, out float OutVignette)
{
    float2 ThisScreen = 2.0 * HitUVz.xy - 1.0; //[-1,1]
    float4 ThisClip = float4(ThisScreen, HitUVz.z, 1);
    float4 PrevClip = mul(ThisClip, ClipToPreClipNoAA);
    float2 PrevScreen = PrevClip.xy / PrevClip.w;

    float2 Velocity = VelocityBuffer.SampleLevel(PointSampler, HitUVz.xy, 0).xy;
    PrevScreen = ThisClip.xy - Velocity;

    float2 PrevUV = 0.5 * PrevScreen.xy + 0.5;

    OutVignette = min(ComputeHitVignetteFromScreenPos(ThisScreen), ComputeHitVignetteFromScreenPos(PrevScreen));
    OutPrevUV = PrevUV;
}