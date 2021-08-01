
#pragma pack_matrix(row_major)

struct VertexOutput
{
    float2 tex : TEXCOORD;
    float4 gl_Position : SV_Position;
};

Texture2D Texture : register(t0);
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
    return Texture.Sample(LinearSampler, float2(1, 1) - pin.tex);
}


