
#pragma pack_matrix(row_major)
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

Texture2D Texture : register(t0);
SamplerState LinearSampler : register(s0);


VertexOutput vs_main(VertexIN IN)
{
    VertexOutput OUT;
    
   // float4 Position = float4(IN.inPos.x * 2.0 - 1.0, 1.0 - 2.0 * IN.inPos.y, 0, 1);
    
    OUT.gl_Position = float4(IN.inPos, 1.0);
    //OUT.gl_Position = Position;
    OUT.tex = IN.tex;
    return OUT;
}


float4 ps_main(VertexOutput pin) : SV_Target
{
    //return float4(1, 0, 1, 1);
    return float4(Texture.Sample(LinearSampler, pin.tex).rgb, 1.0f);
}


