
#pragma pack_matrix(row_major)

struct VertexOutput
{
    float2 tex : TEXCOORD;
    float4 gl_Position : SV_Position;
};

Texture2D AOTexture : register(t0);
Texture2D DOTexture : register(t1);
Texture2D AlbedoTexture : register(t2);
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
    float Ambient = AOTexture.Sample(LinearSampler, pin.tex).r;
    float3 Albedo = AlbedoTexture.Sample(LinearSampler, pin.tex).rgb;
    return float4(Albedo * Ambient + DOTexture.Sample(LinearSampler, pin.tex).rgb,1.0);

}


