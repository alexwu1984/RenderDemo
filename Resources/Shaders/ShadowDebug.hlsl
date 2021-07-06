//***************************************************************************************
// ShadowDebug.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************
#pragma pack_matrix(row_major)
struct VertexIN
{
    float3 inPos : POSITION;
	float2 tex   : TEXCOORD;
    float3 normal : NORMAL;
};

struct VertexOutput
{
	float2 tex			: TEXCOORD;
    float4 gl_Position	: SV_Position;
};

struct ModelViewProjection
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float4x4 viewMatrix;
};


ConstantBuffer<ModelViewProjection> MVP	: register(b0);
Texture2D ShadowTexture				: register(t0);
SamplerState LinearSampler				: register(s0);


VertexOutput vs_main(VertexIN IN)
{
	VertexOutput OUT;
	OUT.gl_Position = mul(float4(IN.inPos, 1.0f), mul(MVP.modelMatrix, mul(MVP.viewMatrix, MVP.projectionMatrix)));
	OUT.tex = IN.tex;
	return OUT;
}


float4 ps_main(VertexOutput pin) : SV_Target
{
    return float4(ShadowTexture.Sample(LinearSampler, pin.tex).rrr, 1.0f);
}


