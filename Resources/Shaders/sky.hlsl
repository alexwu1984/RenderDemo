#pragma pack_matrix(row_major)

struct ModelViewProjection
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;
    float4x4 viewMatrix;
};

ConstantBuffer<ModelViewProjection> MVP : register(b0);
TextureCube gCubeMap : register(t0);
SamplerState LinearSampler : register(s0);

struct VertexIN
{
    float3 inPos : POSITION;
    float2 tex : TEXCOORD;
    float3 normal : NORMAL;
};

struct VertexOutput
{
    float4 gl_Position : SV_Position;
    float3 tex : POSITION;
};

VertexOutput vs_main(VertexIN IN)
{
    VertexOutput OUT;
    OUT.gl_Position = mul(float4(IN.inPos, 1.0f), mul(MVP.modelMatrix, mul(MVP.viewMatrix, MVP.projectionMatrix))).xyww;
    OUT.tex = IN.inPos;
    return OUT;
}

struct PixelOutput
{
    float4 outFragColor : SV_Target0;
};

PixelOutput ps_main(VertexOutput IN)
{
    PixelOutput outColor;
    outColor.outFragColor = gCubeMap.Sample(LinearSampler, IN.tex);
    return outColor;
}


