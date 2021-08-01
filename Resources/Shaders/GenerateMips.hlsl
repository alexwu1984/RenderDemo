#include "ShaderUtils.hlsl"

struct VertexOutput
{
	float2 Tex	: TEXCOORD;
	float4 Pos	: SV_Position;
};

cbuffer PSContant : register(b0)
{
	float2 TexelSize;
	int SrvLevel;
};

Texture2D SrcMipTexture	: register(t0);
SamplerState LinearSampler	: register(s0);

VertexOutput VS_Main(in uint VertID : SV_VertexID)
{
    VertexOutput Output;
	// Texture coordinates range [0, 2], but only [0, 1] appears on screen.
    float2 Tex = float2(uint2(VertID, VertID << 1) & 2);
    Output.Tex = Tex;
    Output.Pos = float4(lerp(float2(-1, 1), float2(1, -1), Tex), 0, 1);
    return Output;
}

float4 PS_Main(in VertexOutput Input) : SV_Target0
{
	float4 c= SrcMipTexture.SampleLevel(LinearSampler, Input.Tex, SrvLevel);
	return c*1.0;
}