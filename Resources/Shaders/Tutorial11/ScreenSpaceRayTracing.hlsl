#pragma pack_matrix(row_major)

struct BasePassInfo
{
    float4x4 ProjectionMatrix;
    float4x4 ModelMatrix;
    float4x4 ViewMatrix;
    int mUseTex ;
    float3 pad;
};

struct SSRInfo
{
    float Near;
    float Far;
    float WindowWidth;
    float WindowHeight;
    float3 CameraPosInWorldSpace;
    float RayLength;
};

struct Ray
{
    float3 Origin;
    float3 Direction;
};

struct Result
{
    bool IsHit;
    float2 UV;
    float3 Position;
    int IterationCount;
};

ConstantBuffer<BasePassInfo> BasePassCBInfo : register(b0);
ConstantBuffer<SSRInfo> SSRCBInfo : register(b1);
Texture2D DepthTexture : register(t0);
Texture2D AlbedoTexture : register(t1);

SamplerState LinearSampler : register(s0);

float LinearizeDepth(float d)
{
    float z = d * 2.0 - 1.0;
    //视线坐标系看向的z轴负方向，因此要求视觉空间的z值应该要把线性深度变成负值
    return -(-SSRCBInfo.Near * SSRCBInfo.Far) / (z * (SSRCBInfo.Far - SSRCBInfo.Near) - SSRCBInfo.Far);
}

float4 projectToScreenSpace(float3 vPoint)
{
    return mul(float4(vPoint, 1), BasePassCBInfo.ProjectionMatrix);
}

float3 projectToViewSpace(float3 vPointInViewSpace)
{
    return mul(float4(vPointInViewSpace, 1), BasePassCBInfo.ViewMatrix).xyz;
}

float2 distanceSquared(float2 A, float2 B)
{
    A -= B;
    return float2(dot(A, A), dot(A, A));
}

bool Query(float2 z, float2 uv)
{
    float TexZ = DepthTexture.Sample(LinearSampler, uv / float2(SSRCBInfo.WindowWidth, SSRCBInfo.WindowHeight)).r;
    float depths = LinearizeDepth(TexZ);
    return z.y < depths && z.x > depths;
}

Result RayMarching(Ray vRay)
{
    Result result;

    float3 Begin = vRay.Origin;
    float3 End = vRay.Origin + vRay.Direction * SSRCBInfo.RayLength;

    float3 V0 = projectToViewSpace(Begin);
    float3 V1 = projectToViewSpace(End);

    float4 H0 = projectToScreenSpace(V0);
    float4 H1 = projectToScreenSpace(V1);

    float k0 = 1.0 / H0.w;
    float k1 = 1.0 / H1.w;

    float3 Q0 = V0 * k0;
    float3 Q1 = V1 * k1;

	// NDC-space not Screen Space
    float2 P0 = H0.xy * k0;
    float2 P1 = H1.xy * k1;
    float2 Size = float2(SSRCBInfo.WindowWidth, SSRCBInfo.WindowHeight);
	//Screen Space
    P0 = (P0 + 1) / 2 * Size;
    P1 = (P1 + 1) / 2 * Size;

    
    P1 += float2((distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);

    float2 Delta = P1 - P0;

    bool Permute = false;
    if (abs(Delta.x) < abs(Delta.y))
    {
        Permute = true;
        Delta = Delta.yx;
        P0 = P0.yx;
        P1 = P1.yx;
    }
    float StepDir = sign(Delta.x);
    float Invdx = StepDir / Delta.x;
    float3 dQ = (Q1 - Q0) * Invdx;
    float dk = (k1 - k0) * Invdx;
    float2 dP = float2(StepDir, Delta.y * Invdx);
    float Stride = 1.0f;

    dP *= Stride;
    dQ *= Stride;
    dk *= Stride;

    P0 += dP;
    Q0 += dQ;
    k0 += dk;
	
    int Step = 0;
    int MaxStep = 5000;
    float k = k0;
    float EndX = P1.x * StepDir;
    float3 Q = Q0;
    float prevZMaxEstimate = V0.z;
    [loop]
    for (float2 P = P0; Step < MaxStep; Step++, P += dP, Q.z += dQ.z, k += dk)
    {
        result.UV = Permute ? P.yx : P;
        float2 Depths;
        Depths.x = prevZMaxEstimate;
        Depths.y = 1-(dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
        prevZMaxEstimate = Depths.y;
        if (Depths.x < Depths.y)
            Depths.xy = Depths.yx;
        if (result.UV.x > SSRCBInfo.WindowWidth || result.UV.x < 0 || result.UV.y > SSRCBInfo.WindowHeight || result.UV.y < 0)
            break;
        result.IsHit = Query(Depths, result.UV);
        if (result.IsHit)
            break;
    }
    return result;
}


struct VertexIN
{
    float3 inPos : POSITION;
    float2 tex : TEXCOORD;
    float3 normal : NORMAL;
};

struct VertexOutput
{
    float2 tex : TEXCOORD;
    float4 gl_Position : SV_Position;
    float4 FragPosInWorldSpace : POSITION;
};

VertexOutput vs_main(VertexIN IN)
{
    VertexOutput OUT;
    OUT.gl_Position = mul(float4(IN.inPos, 1.0f), mul(BasePassCBInfo.ModelMatrix, mul(BasePassCBInfo.ViewMatrix, BasePassCBInfo.ProjectionMatrix)));
    OUT.FragPosInWorldSpace = mul(float4(IN.inPos, 1.0f), BasePassCBInfo.ModelMatrix);
    OUT.tex = IN.tex;
    return OUT;
}

struct PixelOutput
{
    float4 Color : SV_Target0;
};

PixelOutput ps_main(VertexOutput IN)
{
    PixelOutput output;
    
    float3 OrginPoint = IN.FragPosInWorldSpace.xyz;
    float3 ViewDir = normalize(SSRCBInfo.CameraPosInWorldSpace - OrginPoint);
    float3 Normal = float3(0, 1, 0);
    float3 ReflectDir = normalize(reflect(-ViewDir, Normal));
    
    Ray ray;
    ray.Origin = OrginPoint;
    ray.Direction = ReflectDir;
    Result result = RayMarching(ray);
    float3 End = OrginPoint;
    float3 V1 = projectToViewSpace(End);
    float4 H1 = projectToScreenSpace(V1);
    
    float k1 = 1.0 / H1.w;
    
    float3 Q1 = V1 * k1;
    if (result.IsHit)
    {
        output.Color =  AlbedoTexture.Sample(LinearSampler, result.UV / float2(SSRCBInfo.WindowWidth, SSRCBInfo.WindowHeight));
    }
    else
    {
        float4 PointInScreen = mul(mul(float4(OrginPoint, 1), BasePassCBInfo.ViewMatrix), BasePassCBInfo.ProjectionMatrix);
        PointInScreen.xy /= PointInScreen.w;
        PointInScreen.xy = PointInScreen.xy * float2(0.5, 0.5) + float2(0.5, 0.5);
        PointInScreen.y = 1 - PointInScreen.y;
        
        output.Color = AlbedoTexture.Sample(LinearSampler, PointInScreen.xy);

    }

    return output;

}