#pragma pack_matrix(row_major)


struct ModelViewProjection
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float4x4 viewMatrix;
};

struct LightMaterial
{
	float3 uKd;
	float  pad1;
	float3 uKs;
	float  pad2;
	float3 uLightPos;
	float  pad3;
	float3 uCameraPos;
	float  pad4;
	float3 uLightIntensity;
	float  pad5;
};

ConstantBuffer<ModelViewProjection> MVP	: register(b0);
ConstantBuffer<LightMaterial> LightPass	: register(b1);

Texture2D DiffuseTexture				: register(t0);
SamplerState LinearSampler				: register(s0);


struct VertexIN
{
    float3 inPos : POSITION;
	float2 tex   : TEXCOORD;
    //float3 color : COLOR;
};

struct VertexOutput
{
	float2 tex			: TEXCOORD;
    float4 gl_Position	: SV_Position;
    //float3 color		: COLOR;
};

struct PixelOutput
{
    float4 outFragColor : SV_Target0;
};


VertexOutput vs_main(VertexIN IN)
{
	VertexOutput OUT;
	OUT.gl_Position = mul(float4(IN.inPos, 1.0f), mul(MVP.modelMatrix, mul(MVP.viewMatrix, MVP.projectionMatrix)));
	//OUT.color = IN.color;
	OUT.tex = IN.tex;
	return OUT;
}


PixelOutput ps_main(VertexOutput IN)
{
	PixelOutput output;
	//output.outFragColor = float4(IN.color, 1.0f);
	float4 texColor = DiffuseTexture.Sample( LinearSampler, IN.tex );
	output.outFragColor = texColor * float4(LightPass.uKd,1.0);
	//output.outFragColor = float4(IN.color, 1.0f);
	return output;
}