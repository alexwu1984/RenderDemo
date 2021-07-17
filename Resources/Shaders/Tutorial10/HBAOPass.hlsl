#pragma pack_matrix(row_major)

struct AOPassInfo
{
    float Near ;
    float Far ;
    float fov;
    float pad;
    float2 FocalLen;
    float AOStrength;
    float WindowWidth;
    float WindowHeight;
    float MaxRadiusPixels;
    float R;
    float R2;
    float NegInvR2;
    float TanBias;
    float2 pad1;
};

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
};

ConstantBuffer<AOPassInfo> HBAOPass : register(b0);
Texture2D DepthTexture : register(t0);
Texture2D NoiseTexture : register(t1);

SamplerState LinearSampler : register(s0);

float ViewSpaceZFromDepth(float d)
{
    float z = d * 2.0 - 1.0;
    //视线坐标系看向的z轴负方向，因此要求视觉空间的z值应该要把线性深度变成负值
    return -(-HBAOPass.Near * HBAOPass.Far) / (z * (HBAOPass.Far - HBAOPass.Near) - HBAOPass.Far);
}

float3 UVToViewSpace(float2 uv, float z)
{
    uv = uv * 2.0 - 1.0;
    uv.x = uv.x * tan(HBAOPass.fov / 2.0) * HBAOPass.WindowHeight / HBAOPass.WindowHeight * z;
    uv.y = uv.y * tan(HBAOPass.fov / 2.0) * z;
    return float3(-uv, z);
}

float3 GetViewPos(float2 uv)
{
    float z = ViewSpaceZFromDepth(DepthTexture.Sample(LinearSampler, uv).r);
    return UVToViewSpace(uv, z);
}

float TanToSin(float x)
{
    return x * rsqrt(x * x + 1.0);
}

float InvLength(float2 V)
{
    return rsqrt(dot(V, V));
}

float Tangent(float3 V)
{
    return V.z * InvLength(V.xy);
}

float BiasedTangent(float3 V)
{
    return V.z * InvLength(V.xy) + HBAOPass.TanBias;
}

float Tangent(float3 P, float3 S)
{
    return Tangent(S - P);
}

float Length2(float3 V)
{
    return dot(V, V);
}

float3 MinDiff(float3 P, float3 Pr, float3 Pl)
{
    float3 V1 = Pr - P;
    float3 V2 = P - Pl;
    return (Length2(V1) < Length2(V2)) ? V1 : V2;
}

float2 SnapUVOffset(float2 uv)
{
    return round(uv * float2(HBAOPass.WindowWidth, HBAOPass.WindowHeight)) * float2(1.0 / HBAOPass.WindowWidth, 1.0 / HBAOPass.WindowHeight);
}

float Falloff(float d2)
{
    return d2 * HBAOPass.NegInvR2 + 1.0f;
}

float HorizonOcclusion(float2 TexCoord,
                        float2 deltaUV,
						float3 P,
						float3 dPdu,
						float3 dPdv,
						float randstep,
						float numSamples)
{
    float ao = 0;
    float2 uv = TexCoord + SnapUVOffset(randstep * deltaUV);
    deltaUV = SnapUVOffset(deltaUV);
    float3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;
    float tanH = BiasedTangent(T);
    float sinH = TanToSin(tanH);
    float tanS;
    float d2;
    float3 S;
    numSamples = min(numSamples, 10);
    [loop]
    for (float s = 1; s <= numSamples; ++s)
    {
        uv += deltaUV;
        S = GetViewPos(uv);
        tanS = Tangent(P, S);
        d2 = Length2(S - P);
        if (d2 < HBAOPass.R2 && tanS > tanH)
        {
            float sinS = TanToSin(tanS);
            ao += Falloff(d2) * (sinS - sinH);
            tanH = tanS;
            sinH = sinS;
        }
    }
    return ao;
}

float2 RotateDirections(float2 Dir, float2 CosSin)
{
    return float2(Dir.x * CosSin.x - Dir.y * CosSin.y,
                  Dir.x * CosSin.y + Dir.y * CosSin.x);
}

void ComputeSteps(inout float2 stepSizeUv, inout float numSteps, float rayRadiusPix, float rand, float numSamples)
{
    numSteps = min(numSamples, rayRadiusPix);
    numSteps = min(numSteps, 10);
    float stepSizePix = rayRadiusPix / (numSteps + 1);

    float maxNumSteps = HBAOPass.MaxRadiusPixels / stepSizePix;
    if (maxNumSteps < numSteps)
    {
        numSteps = floor(maxNumSteps + rand);
        numSteps = max(numSteps, 1);
        stepSizePix = HBAOPass.MaxRadiusPixels / numSteps;
    }

    stepSizeUv = stepSizePix * float2(1.0 / HBAOPass.WindowHeight, 1.0 / HBAOPass.WindowHeight);
}

VertexOutput vs_main(in uint VertID : SV_VertexID)
{
    VertexOutput Output;
	// Texture coordinates range [0, 2], but only [0, 1] appears on screen.
    float2 Tex = float2(uint2(VertID, VertID << 1) & 2);
    Output.tex = Tex;
    Output.gl_Position = float4(lerp(float2(-1, 1), float2(1, -1), Tex), 0, 1);
    return Output;
}

float4 ps_main(VertexOutput pin) : SV_Target
{
    float2 NoiseScale = float2(HBAOPass.WindowWidth / 4.0f, HBAOPass.WindowHeight / 4.0f);
    const float numDirections = 6;
    const float PI = 3.14159265;
    
    float3 P, Pr, Pl, Pt, Pb;
    P = GetViewPos(pin.tex);
    Pr = GetViewPos(pin.tex + float2(1.0 / HBAOPass.WindowWidth, 0));
    Pl = GetViewPos(pin.tex + float2(-1.0 / HBAOPass.WindowWidth, 0));
    Pt = GetViewPos(pin.tex + float2(0, 1.0 / HBAOPass.WindowHeight));
    Pb = GetViewPos(pin.tex + float2(0, -1.0 / HBAOPass.WindowHeight));
    float3 dPdu = MinDiff(P, Pr, Pl);
    float3 dPdv = MinDiff(P, Pt, Pb) * (HBAOPass.WindowHeight * 1.0 / HBAOPass.WindowWidth);
    
    float3 leftDir = min(P - Pl, Pr - P) ? P - Pl : Pb - P; //求出最小的变换量
    float3 upDir = min(P - Pb, Pt - P) ? P - Pb : Pt - P; //求出最小的变换量
    float3 normal = normalize(cross(leftDir, upDir));
    
    float3 random = NoiseTexture.Sample(LinearSampler, pin.tex * NoiseScale).rgb;
    float2 rayRadiusUV = 0.5 * HBAOPass.R * HBAOPass.FocalLen / -P.z;
    float rayRadiusPix = rayRadiusUV.x * HBAOPass.WindowWidth;
    float ao = 1.0;
    float numSteps;
    float2 stepSizeUV;
    if (rayRadiusPix > 1.0)
    {
        ao = 0.0;
        ComputeSteps(stepSizeUV, numSteps, rayRadiusPix, random.z, 3);
        float alpha = 2.0 * PI / numDirections;
        [loop]
        for (float d = 0; d < numDirections; ++d)
        {
            float theta = alpha * d;
            float2 dir = RotateDirections(float2(cos(theta), sin(theta)), random.xy);
            float2 deltaUV = dir * stepSizeUV;
            ao += HorizonOcclusion(pin.tex, deltaUV, P, dPdu, dPdv, random.z, numSteps);
        }
        ao = 1.0 - ao / numDirections * HBAOPass.AOStrength;
    }
    return float4(ao,0,0, 1.0);

}