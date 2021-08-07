#pragma pack_matrix(row_major)

#include "brdf.hlsl"

cbuffer VSContant : register(b0)
{
	float4x4 ModelMatrix;
	float4x4 ViewProjMatrix;
};

cbuffer PSContant : register(b0)
{
    float Exposure;
    int MipLevel;
    int MaxMipLevel;
    int NumSamplesPerDir;
    int Degree;
    float3	CameraPos;
    int		bSHDiffuse;
    float3	pad;
    float4	Coeffs[16];
    float3 LightDir;
    int EnableLight;
};

Texture2D BaseMap 			: register(t0);
Texture2D OpacityMap 		: register(t1);
Texture2D EmissiveMap 		: register(t2);
Texture2D MetallicMap 		: register(t3);
Texture2D RoughnessMap 		: register(t4);
Texture2D AOMap 			: register(t5);
Texture2D NormalMap 		: register(t6);
TextureCube IrradianceCubeMap : register(t7);
TextureCube PrefilteredCubeMap: register(t8);
Texture2D PreintegratedGF 	: register(t9);

SamplerState LinearSampler	: register(s0);


struct VertexInput
{
	float3 Position : POSITION;
	float2 Tex		: TEXCOORD;
	float3 Normal	: NORMAL;
	float4 Tangent	: TANGENT;
};

struct PixelInput
{
	float4 Position	: SV_Position;
	float2 Tex		: TEXCOORD0;
	float3 T		: TEXCOORD1;
	float3 B		: TEXCOORD2;
	float3 N		: TEXCOORD3;
	float3 WorldPos	: TEXCOORD4;
};

struct FDirectLighting
{
    float3 Diffuse;
    float3 Specular;
    float3 Transmission;
};


float3 F_schlickR(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max(1.0 - roughness, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

PixelInput VS_PBR(VertexInput In)
{
	PixelInput Out;
	Out.Tex = In.Tex;

	float4 WorldPos = mul(float4(In.Position, 1.0), ModelMatrix);
	Out.WorldPos = WorldPos.xyz;
	Out.Position = mul(WorldPos, ViewProjMatrix);

	Out.N = mul(In.Normal, (float3x3)ModelMatrix);
    Out.T = mul(In.Tangent.xyz, (float3x3) ModelMatrix);
	Out.B = cross(In.Normal, In.Tangent.xyz) * In.Tangent.w;
	Out.B = mul(Out.B, (float3x3)ModelMatrix);
    return Out;
}

float4 PS_PBR(PixelInput In) : SV_Target
{
    float Opacity = OpacityMap.Sample(LinearSampler, In.Tex).r;
    clip(Opacity < 0.1f ? -1 : 1);

    float3 Albedo = BaseMap.Sample(LinearSampler, In.Tex).xyz;
    float Metallic = MetallicMap.Sample(LinearSampler, In.Tex).x;
    float Roughness = RoughnessMap.Sample(LinearSampler, In.Tex).x;

    float3x3 TBN = float3x3(normalize(In.T), normalize(In.B), normalize(In.N));
    float3 tNormal = NormalMap.Sample(LinearSampler, In.Tex).xyz;
    tNormal = 2 * tNormal - 1.0; // [0,1] -> [-1, 1]
    float3 N = mul(tNormal, TBN);

    float3 V = normalize(CameraPos - In.WorldPos);
    float3 R = reflect(-V, N); //incident ray, surface normal

    float NoV = saturate(dot(N, V));
    float3 F0 = lerp(0.04, Albedo.rgb, Metallic);
    float3 F = F_schlickR(NoV, F0, Roughness);

    float3 kD = (1.0 - F) * (1.0 - Metallic);
    float3 Irradiance = IrradianceCubeMap.SampleLevel(LinearSampler, N, 0).xyz;
    float3 Diffuse = Albedo * kD ;
    
    FDirectLighting Lighting;
    Lighting.Diffuse = 0;
    Lighting.Specular = 0;
    if (EnableLight == 1)
    {
        float3 L = -LightDir;
        BxDFContext Context;
        Init(Context, N, V, L);
        Context.NoL = saturate(Context.NoL);
        Context.NoV = saturate(abs(Context.NoV) + 1e-5);
        
        float3 Color = 0;

	// Diffuse
        Lighting.Diffuse = Context.NoL * Diffuse_Burley(Diffuse, Roughness, Context.NoV, Context.NoL, Context.VoH);
        Lighting.Specular = Context.NoL * SpecularGGX(Roughness, F0, Context, Context.NoL);
        
        Diffuse = Lighting.Diffuse;

    }
    else
    {
 
        Diffuse = Diffuse_Lambert(Diffuse);
        Diffuse *= Irradiance;
    }

    float Mip = ComputeReflectionCaptureMipFromRoughness(Roughness, MaxMipLevel - 1);
    float2 BRDF = PreintegratedGF.Sample(LinearSampler, float2(NoV, Roughness)).rg;

	
    float3 PrefilteredColor = PrefilteredCubeMap.SampleLevel(LinearSampler, R, Mip).rgb;
    float3 Specular = PrefilteredColor * (F * BRDF.x + BRDF.y);

    float3 Emissive = EmissiveMap.Sample(LinearSampler, In.Tex).xyz;
    float3 FinalColor = Emissive + Diffuse + Specular + Lighting.Specular;

    return float4(ToneMapping(FinalColor * Exposure), Opacity);
}