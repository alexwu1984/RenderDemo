#pragma pack_matrix(row_major)

struct SSDOPassInfo
{
    float4 Samples[64];
    float WindowWidth;
    float WindowHeight;
    int KernelSize;
    float Radius;
    float IndirectLightScale;
    float3 pad;
    float4x4 ProjectionMatrix;
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

ConstantBuffer<SSDOPassInfo> SSDOPass : register(b0);
Texture2D PositionTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D NoiseTexture : register(t2);
Texture2D LightTexture : register(t3);
SamplerState LinearSampler : register(s0);

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
    float2 NoiseScale = float2(SSDOPass.WindowWidth / 4.0f, SSDOPass.WindowHeight / 4.0f);
    float3 FragPos = PositionTexture.Sample(LinearSampler, pin.tex).xyz;
    float3 Normal = NormalTexture.Sample(LinearSampler, pin.tex).rgb;
    float3 RandomVec = NoiseTexture.Sample(LinearSampler, pin.tex*NoiseScale).xyz;
    float3 Tangent = normalize(RandomVec - Normal * dot(RandomVec, Normal)); //计算切线(Normal * dot(RandomVec, Normal)是 RandomVec 在Normal向量上的投影
    float3 Bitangent = cross(Normal,Tangent);//计算副法线
    float3x3 TBN = float3x3(Tangent, Bitangent,Normal);
    float3 IndirectLight = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < SSDOPass.KernelSize; ++i)
    {
        float3 Samples = mul(SSDOPass.Samples[i].xyz, TBN);
        Samples = FragPos + Samples * SSDOPass.Radius;
        float4 Offset = float4(Samples,1.0);
        Offset = mul(Offset, SSDOPass.ProjectionMatrix);
        Offset.xyz /= Offset.w;
        
        Offset.x = Offset.x * 0.5 + 0.5;
        Offset.y = Offset.y * -0.5 + 0.5;
        Offset.z = Offset.z * 0.5 + 0.5;
        
        float SampleDepth = PositionTexture.Sample(LinearSampler, Offset.xy).z;
        float3 SampleNormal = NormalTexture.Sample(LinearSampler, Offset.xy).rgb;
        float3 SamplePos = PositionTexture.Sample(LinearSampler, Offset.xy).xyz;
        float3 SampleColor = LightTexture.Sample(LinearSampler, Offset.xy).rgb;
        float3 RangeCheck = max(dot(SampleNormal, normalize(FragPos - SamplePos)), 0.0) * SampleColor;
        IndirectLight += (SampleDepth <= Samples.z ? 1.0 : 0.0) * RangeCheck;

    }
    IndirectLight = IndirectLight / SSDOPass.KernelSize * 5.0;
    return float4(IndirectLight, 1.0);

}