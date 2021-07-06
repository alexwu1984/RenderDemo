#pragma pack_matrix(row_major)


struct ModelViewProjection
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float4x4 viewMatrix;
	int UseTex;
	int3 pad;
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
    float4 Flux : SV_Target0;
    float4 Normal : SV_Target1;
    float4 Position : SV_Target2;
};

struct LightMaterial
{
	float3 uKd;
	float  pad1;
	float3 uKs;
	float  pad2;
	float3 uLightDir;
	float  pad4;
	float3 uCameraPos;
	float  pad5;
	float3 uLightIntensity;
	int  shadowType;
    float4 uLightColor;
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float4x4 viewMatrix;
};

ConstantBuffer<ModelViewProjection> BasePass : register(b0);
ConstantBuffer<LightMaterial> LightPass	: register(b1);
Texture2D DiffuseTexture : register(t0);
SamplerState LinearSampler : register(s0);

VertexOutput vs_main(VertexIN IN)
{
    
 //   vec4 FragPosInWorldSpace = u_ModelMatrix * vec4(_Position, 1.0f);
 //   gl_Position = u_LightVPMatrix * FragPosInWorldSpace;
 //   v2f_TexCoords = _TexCoord;
	////存储的是在相机空间下的位置以及法线，不是光源空间下的
 //   v2f_Normal = normalize(mat3(transpose(inverse(u_ViewMatrix * u_ModelMatrix))) * _Normal); //这个可以在外面算好了传进来
 //   v2f_FragPosInViewSpace = vec3(u_ViewMatrix * FragPosInWorldSpace);
    VertexOutput OUT;
    float4 FragPosInWorldSpace = mul(float4(IN.inPos, 1.0f), BasePass.modelMatrix);
    OUT.gl_Position = mul(FragPosInWorldSpace, mul(LightPass.viewMatrix, LightPass.projectionMatrix));
   // OUT.gl_Position = mul(float4(IN.inPos, 1.0f), mul(LightPass.modelMatrix, mul(LightPass.viewMatrix, LightPass.projectionMatrix)));
    OUT.tex = IN.tex;
    OUT.normalw = mul(IN.normal, (float3x3) BasePass.modelMatrix);
    OUT.FragPosInViewSpace = FragPosInWorldSpace;//
    
    //OUT.gl_Position = mul(float4(IN.inPos, 1.0f), mul(BasePass.modelMatrix, mul(BasePass.viewMatrix, BasePass.projectionMatrix)));
    //OUT.FragPosInViewSpace = mul(float4(IN.inPos, 1.0f), mul(LightPass.modelMatrix, LightPass.viewMatrix));
    //OUT.tex = IN.tex;
    //OUT.normalw = mul(IN.normal, (float3x3) BasePass.modelMatrix);
    
    return OUT;
}

PixelOutput ps_main(VertexOutput IN)
{
 //   vec3 TexelColor = texture(u_DiffuseTexture, v2f_TexCoords).rgb;
	////TexelColor = pow(TexelColor, vec3(2.2f));
 //   vec3 VPLFlux = u_LightColor * TexelColor;
 //   Flux_ = VPLFlux;
 //   Normal_ = v2f_Normal;
 //   Position_ = v2f_FragPosInViewSpace;
    
    PixelOutput output;
	
    float4 litColor;
    if (BasePass.UseTex == 1)
    {
        litColor = DiffuseTexture.Sample(LinearSampler, IN.tex);
    }
    else
    {
        litColor = float4(LightPass.uKd, 1.0);
    }
    output.Flux = float4(LightPass.uLightColor.rgb * litColor.rgb, 1.0);
    output.Normal = float4(IN.normalw, 1.0);
    output.Position = IN.FragPosInViewSpace;
    return output;
}
