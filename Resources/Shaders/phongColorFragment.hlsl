#include "shadowcommon.hlsl"
#pragma pack_matrix(row_major)

struct ModelViewProjection
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    int UseTex;
    int3 pad;
};

struct LightMaterial
{
    float3 uKd;
    float pad1;
    float3 uKs;
    float pad2;
    float3 uLightDir;
    float pad4;
    float3 uCameraPos;
    float pad5;
    float3 uLightIntensity;
    int shadowType;
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
    float3 lightColor;
};

ConstantBuffer<ModelViewProjection> MVP	: register(b0);
ConstantBuffer<LightMaterial> LightPass	: register(b1);

struct VertexIN
{
    float3 inPos : POSITION;
	float2 tex   : TEXCOORD;
	float3 normal : NORMAL;
};

struct VertexOutput
{
	float2 tex : TEXCOORD;
	float4 gl_Position : SV_Position;
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


PixelOutput ps_main(VertexOutput IN)
{
    PixelOutput output;

    float shadowFactor = 1.0;
    
    float3 shadowCoord = IN.positionFromLight.xyz / IN.positionFromLight.w;
    shadowCoord.x = shadowCoord.x * 0.5 + 0.5;
    shadowCoord.y = shadowCoord.y * -0.5 + 0.5;
    
    if (LightPass.shadowType == 1)
        shadowFactor = ComputeShadow(shadowCoord, IN.normalw, LightPass.uLightDir);
    else if (LightPass.shadowType == 2)
        shadowFactor = ComputeShadow_VSM(shadowCoord, normalize(IN.normalw), normalize(LightPass.uLightDir));
    
    float3 toEyeW = normalize(LightPass.uCameraPos - IN.gl_Position.xyz);

    float4 ambient = float4(LightPass.uKd, 1.0);

    Material mat = { float4(1.0, 1.0, 1.0, 1.0), float3(0.1, 0.1, 0.1), 0.7,LightPass.uLightColor.rgb };
    
    float4 directLight = float4(ComputeDirectionalLight(mat, IN.normalw, toEyeW)*shadowFactor , 1.0);
    float4 litColor = ambient  + directLight;
    
    output.outFragColor = litColor;
    return output;
}