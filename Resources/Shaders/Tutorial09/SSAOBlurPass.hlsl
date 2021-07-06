#pragma pack_matrix(row_major)


struct VertexOutput
{
    float2 tex : TEXCOORD;
    float4 gl_Position : SV_Position;
};

Texture2D SSAOTexture : register(t0);
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

float4 ps_main(VertexOutput PIN) : SV_Target
{
    float2 SSAOTexSize;
    SSAOTexture.GetDimensions(SSAOTexSize.x, SSAOTexSize.y);
    float2 TexelSize = 1.0f / SSAOTexSize;
    float Result = 0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            float2 Offset = float2(x, y) * TexelSize;
            Result += SSAOTexture.Sample(LinearSampler, PIN.tex + Offset).r;
        }
    }
    Result = Result / (4.0 * 4.0);
    return float4(Result, Result, Result, 1.0);

}