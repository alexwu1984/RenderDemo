#pragma pack_matrix(row_major)

struct ModelViewProjection
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float4x4 viewMatrix;
};

ConstantBuffer<ModelViewProjection> MVP	: register(b0);

struct VertexInput
{
	float3 inPos : POSITION;
	float2 tex: TEXCOORD;
	float3 inColor : NORMAL;
};

struct VertexOutput
{
	float3 outColor : COLOR;
	float4 gl_Position : SV_Position;
};


struct PixelOutput
{
	float4 outFragColor : SV_Target0;
};


VertexOutput vs_main(VertexInput stage_input)
{
	VertexOutput stage_output;
	stage_output.gl_Position = mul(float4(stage_input.inPos, 1.0f), mul(MVP.modelMatrix, mul(MVP.viewMatrix, MVP.projectionMatrix)));
	stage_output.outColor = stage_input.inColor;
	return stage_output;
}


PixelOutput ps_main(VertexOutput stage_input)
{
	PixelOutput stage_output;
	stage_output.outFragColor = float4(stage_input.outColor, 1.0f);
	return stage_output;
}