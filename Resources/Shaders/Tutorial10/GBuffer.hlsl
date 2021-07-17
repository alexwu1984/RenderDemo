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


float LinearizeDepth(float vDepth)
{
    //float z = vDepth * 2.0 - 1.0; 
    //return (-Project.fNear * Project.fFar) / (z * (Project.fFar - Project.fNear) - Project.fFar);
    
    float d = vDepth * 2.0 - 1.0;
	 //视线坐标系看向的z轴负方向，因此要求视觉空间的z值应该要把线性深度变成负值
    return (2.0 * Project.fNear * Project.fFar) / (Project.fFar + Project.fNear - d * (Project.fFar - Project.fNear));
}

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
    output.DiffuseAlbedo = DiffuseTexture.Sample(LinearSampler, IN.tex);;
    output.Normal = float4(IN.normalw, 1.0);
    output.Position = float4(IN.FragPosInViewSpace.xyz, IN.FragPosInViewSpace.z);
    return output;
}