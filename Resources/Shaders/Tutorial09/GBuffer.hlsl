#pragma pack_matrix(row_major)

struct ModelViewProjection
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    int UseTex;
    int3 pad;
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

struct LightMaterial
{
	float3 uKd;
	float  pad1;
	float3 uKs;
	float  pad2;
	float3 uLightDir;
	float  pad4;
	float3 uCameraPos;
	float  pad5;
	float3 uLightIntensity;
	int shadowType;
    float4 uLightColor;
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float4x4 viewMatrix;
};

ConstantBuffer<ModelViewProjection> BasePass : register(b0);
ConstantBuffer<LightMaterial> LightPass	: register(b1);
Texture2D DiffuseTexture : register(t0);
SamplerState LinearSampler : register(s0);

#define g_Near  0.1
#define g_Far 1000.0f

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
    float3 g_LightPos = float3(-18.5264, 19.4874, -78.5421);
    float4 litColor = float4(0, 0, 0, 1);
    float3 LightPosInViewSpace = mul(g_LightPos, (float3x3) mul(BasePass.modelMatrix, BasePass.viewMatrix));
    float3 Dis = LightPosInViewSpace - IN.FragPosInViewSpace.xyz;
    
    if (BasePass.UseTex == 1)
    {
        litColor = DiffuseTexture.Sample(LinearSampler, IN.tex);
    }
    else
    {
        litColor = float4(LightPass.uKd, 1.0);
    }
    litColor *= max(dot(IN.normalw, normalize(Dis)), 0);

    output.DiffuseAlbedo = litColor;
    output.Normal = float4(IN.normalw, 1.0);
    output.Position = IN.FragPosInViewSpace;
    return output;
}