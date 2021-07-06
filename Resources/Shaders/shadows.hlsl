//***************************************************************************************
// Shadows.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************
#pragma pack_matrix(row_major)


struct ModelViewProjection
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;
    float4x4 viewMatrix;
};

ConstantBuffer<ModelViewProjection> MVP : register(b0);

struct VertexIN
{
    float3 inPos : POSITION;
    float2 tex : TEXCOORD;
    float3 normal : NORMAL;
};

struct VertexOutput
{
    float4 gl_Position : SV_Position;
};

VertexOutput vs_main(VertexIN IN)
{
    VertexOutput OUT;
    OUT.gl_Position = mul(float4(IN.inPos, 1.0f), mul(MVP.modelMatrix, mul(MVP.viewMatrix, MVP.projectionMatrix)));
    return OUT;
}


// This is only used for alpha cut out geometry, so that shadows 
// show up correctly.  Geometry that does not need to sample a
// texture can use a NULL pixel shader for depth pass.
void ps_main(VertexOutput pin)
{

}

struct PixelOutput
{
    float4 outFragColor : SV_Target0;
};

PixelOutput ps_main_sqrtshadow(VertexOutput IN)
{
    PixelOutput outColor;
    outColor.outFragColor = float4(IN.gl_Position.z, IN.gl_Position.z * IN.gl_Position.z, IN.gl_Position.z, IN.gl_Position.z * IN.gl_Position.z);
    return outColor;
}


