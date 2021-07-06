
#pragma pack_matrix(row_major)


struct BasePassInfo
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float4x4 viewMatrix;
};

struct SHInfo
{
    float4 Coef[16];
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

struct Material
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Shininess;
};


ConstantBuffer<BasePassInfo> MVP : register(b0);
ConstantBuffer<LightMaterial> LightPass	: register(b1);
ConstantBuffer<SHInfo> SHPass : register(b2);

Texture2D DiffuseTexture : register(t0);
TextureCube CubeMapTex1 : register(t1);
TextureCube CubeMapTex2 : register(t2);
TextureCube CubeMapTex3 : register(t3);

SamplerState LinearSampler : register(s0);

struct VertexIN
{
    float3 inPos : POSITION;
	float2 tex   : TEXCOORD;
	float3 normal : NORMAL;
};

struct VertexOutput
{
	float2 tex			: TEXCOORD;
    float4 gl_Position	: SV_Position;
	float4 positionFromLight : POSITION;
	float3 normalw : NORMAL;
};

struct PixelOutput
{
    float4 outFragColor : SV_Target0;
};

float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);

    return reflectPercent;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
    const float m = mat.Shininess * 256.0f;
    float3 halfVec = normalize(toEye + lightVec);

    float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, halfVec, lightVec);

    float3 specAlbedo = fresnelFactor * roughnessFactor;

    // Our spec formula goes outside [0,1] range, but we are 
    // doing LDR rendering.  So scale it down a bit.
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float3 ComputeDirectionalLight(Material mat, float3 normal, float3 toEye)
{
    // The light vector aims opposite the direction the light rays travel.
    float3 lightVec = -LightPass.uLightDir;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = LightPass.uLightIntensity * ndotl;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}


VertexOutput vs_main(VertexIN IN)
{
	VertexOutput OUT;
	OUT.gl_Position = mul(float4(IN.inPos, 1.0f), mul(MVP.modelMatrix, mul(MVP.viewMatrix, MVP.projectionMatrix)));
	OUT.positionFromLight = mul(float4(IN.inPos, 1.0f), mul(LightPass.modelMatrix, mul(LightPass.viewMatrix, LightPass.projectionMatrix)));
	OUT.tex = IN.tex;
	OUT.normalw = mul(IN.normal, (float3x3) MVP.modelMatrix);
	return OUT;
}

const float PI = 3.1415926535897932384626433832795;

PixelOutput ps_main(VertexOutput IN)
{
	PixelOutput output;
    
    float3 shadowCoord = IN.positionFromLight.xyz / IN.positionFromLight.w;
    shadowCoord.x = shadowCoord.x * 0.5 + 0.5;
    shadowCoord.y = shadowCoord.y * -0.5 + 0.5;
    float shadowFactor = 1.0;
    
    float3 toEyeW = normalize(LightPass.uCameraPos - IN.gl_Position.xyz);
 
    float4 diffuseAlbedo = DiffuseTexture.Sample(LinearSampler, IN.tex);
    
    float4 ambient = float4(0.3, 0.3, 0.3, 1.0) * diffuseAlbedo;

    Material mat = { float4(0.4, 0.4, 0.4, 1.0), float3(0.1, 0.1, 0.1), 0.5 };
    float4 directLight = float4(ComputeDirectionalLight(mat, IN.normalw, toEyeW) * shadowFactor, 1.0);
    float4 litColor = ambient  + directLight;
    
    float3 N = normalize(IN.normalw);
    float3 basis1 = CubeMapTex1.Sample(LinearSampler, N);
    float3 basis2 = CubeMapTex2.Sample(LinearSampler, N);
    float3 basis3 = CubeMapTex3.Sample(LinearSampler, N);
    
    float basis[9];
    basis[0] = basis1.x / PI;
    basis[1] = basis1.y / PI;
    basis[2] = basis1.z / PI;
    basis[3] = basis2.x / PI;
    basis[4] = basis2.y / PI;
    basis[5] = basis2.z / PI;
    basis[6] = basis3.x / PI;
    basis[7] = basis3.y / PI;
    basis[8] = basis3.z / PI;

    float3 color = float3(0, 0, 0);
    for (int i = 0; i < 9; i++)
        color += SHPass.Coef[i].rgb * basis[i];
	
    output.outFragColor = float4(/*litColor.rgb+*/color, 1.0);
	return output;
}