#pragma pack_matrix(row_major)

struct ModelViewProjection
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    int UseTex;
    int3 pad;
};

struct ProjectInfo
{
    float fNear;
    float fFar;
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
    float4 FragPosInViewSpace : POSITION;
    float3 normalw : NORMAL;
};

struct PixelOutput
{
    float4 DiffuseAlbedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 Position : SV_Target2;
};


ConstantBuffer<ModelViewProjection> BasePass : register(b0);
ConstantBuffer<ProjectInfo> Project : register(b1);
Texture2D DiffuseTexture : register(t0);
SamplerState LinearSampler : register(s0);

VertexOutput vs_main(VertexIN IN)
{
	VertexOutput OUT;
    OUT.gl_Position = mul(float4(IN.inPos, 1.0f), mul(BasePass.modelMatrix, mul(BasePass.viewMatrix, BasePass.projectionMatrix)));
    OUT.FragPosInViewSpace = mul(float4(IN.inPos, 1.0f), mul(BasePass.modelMatrix, BasePass.viewMatrix));
	OUT.tex = IN.tex;
    OUT.normalw = mul(IN.normal, (float3x3) BasePass.modelMatrix);
	return OUT;
}


PixelOutput ps_main(VertexOutput IN)
{
	PixelOutput output;
    
    float gamma = 2.2;
    float InvGamma = 1.0 / 2.2;
    float3 diffuseColor = pow(DiffuseTexture.Sample(LinearSampler, IN.tex).rgb, float3(gamma, gamma, gamma));
    float3 mapped = float3(1.0,1.0,1.0) - exp(-diffuseColor);
    mapped = pow(mapped, float3(InvGamma, InvGamma, InvGamma));
    output.DiffuseAlbedo = float4(mapped, 1.0);
   // float alpha = textureLod(u_DiffuseTexture, v2f_TexCoords, 0).a;
    float alpha = DiffuseTexture.Load(float3(IN.tex,0)).a;
    if (alpha != 1.0f)
        discard;
    
    output.Normal = float4(IN.normalw, 1.0);
    output.Position = float4(IN.FragPosInViewSpace.xyz, IN.FragPosInViewSpace.z);
    return output;
}