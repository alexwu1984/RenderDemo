#pragma pack_matrix(row_major)
#include "ShaderUtils.hlsl"

cbuffer VSContant : register(b0)
{
	float4x4 ModelMatrix;
	float4x4 ViewProjMatrix;
};

Texture2D LongLatEnvironment 	: register(t0);
SamplerState LinearSampler		: register(s0);


struct VertexIN
{
	float3 Position : POSITION;
};

struct VertexOutput
{
	float4 Position	 	  : SV_Position;
	float3 LocalDirection : TEXCOORD;
};
//-------------------------------------------------------
// Convert Longitude-Latitude Mapping to Cube Mapping
//-------------------------------------------------------

VertexOutput VS_LongLatToCube(VertexIN In)
{
	VertexOutput Out;
	Out.LocalDirection = In.Position;
	Out.Position = mul(mul(float4(In.Position, 1.0), ModelMatrix), ViewProjMatrix);
	return Out;
}

static const float2 invAtan = { 0.5 / PI, -1 / PI };
float2 SampleSphericalMap(float3 Direction)
{
	float3 v = normalize(Direction);
	float2 uv = { atan2(v.z, v.x), asin(v.y) };
	uv = saturate(uv * invAtan + 0.5);
	return uv;
}


float4 PS_LongLatToCube(VertexOutput In) : SV_Target
{
	return LongLatEnvironment.Sample(LinearSampler, SampleSphericalMap(In.LocalDirection.xyz));
}

//-------------------------------------------------------
// SkyBox
//-------------------------------------------------------

VertexOutput VS_SkyCube(VertexIN In)
{
    VertexOutput Out;
    Out.LocalDirection = In.Position;
    Out.Position = mul(mul(float4(In.Position, 1.0), ModelMatrix), ViewProjMatrix).xyww;
    return Out;
}

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
};

TextureCube CubeEnvironment : register(t0);

float4 PS_SkyCube(VertexOutput In) : SV_Target
{
	// Local Direction don't need to normalized
    float3 Color = CubeEnvironment.Sample(LinearSampler, In.LocalDirection).xyz;

    return float4(ToneMapping(Color * Exposure), 1.0);
}

//-------------------------------------------------------
// Generate Irradiance map
//-------------------------------------------------------
float4 PS_GenIrradiance(VertexOutput In) : SV_Target
{
    float3 Normal = normalize(In.LocalDirection);
    float3 Irradiance = { 0.0, 0.0, 0.0 };

    float3 Up = { 0.0, 1.0, 0.0 };
    float3 Right = cross(Up, Normal);
    Up = cross(Normal, Right);

    float sampleDelta = 1.0 / NumSamplesPerDir;

    uint2 Dimension;
    CubeEnvironment.GetDimensions(Dimension.x, Dimension.y);
    float lod = max(log2(Dimension.x / float(NumSamplesPerDir)) + 1.0, 0.0);

    float NumSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
			// spherical to cartesian (in tangent space)
            float sintheta = sin(theta);
            float costheta = cos(theta);
            float3 tangentSample = float3(sintheta * cos(phi), sintheta * sin(phi), costheta);
			// tangent space to world
			//float3 sampleVec = tangentSample.x * Right + tangentSample.y * Up + tangentSample.z * Normal;
            float3 sampleVec = TangentToWorld(tangentSample, Normal);
            float3 sampleColor = CubeEnvironment.SampleLevel(LinearSampler, sampleVec, lod).rgb;

            Irradiance += sampleColor * costheta * sintheta;
            NumSamples++;
        }
    }

    Irradiance = PI * Irradiance / NumSamples;

    return float4(Irradiance, 1.0);
}

//-------------------------------------------------------
// Show Texture2D
//-------------------------------------------------------

struct VertexOutput_Texture2D
{
    float4 Pos : SV_Position;
    float2 Tex : TEXCOORD;
};

Texture2D InputTexture : register(t0);

VertexOutput_Texture2D VS_ShowTexture2D(in uint VertID : SV_VertexID)
{
    VertexOutput_Texture2D Output;
	// Texture coordinates range [0, 2], but only [0, 1] appears on screen.
    float2 Tex = float2(uint2(VertID, VertID << 1) & 2);
    Output.Tex = Tex;
    Output.Pos = float4(lerp(float2(-1, 1), float2(1, -1), Tex), 0, 1);
    return Output;
}

float4 PS_ShowTexture2D(in VertexOutput_Texture2D In) : SV_Target0
{
    float3 Color = InputTexture.Sample(LinearSampler, In.Tex).xyz;

    return float4(ToneMapping(Color * Exposure), 1.0);
}

//-------------------------------------------------------
// CubeMap Cross View
//-------------------------------------------------------

struct VertexIN_CubeMapCross
{
    float3 Position : POSITION;
    float3 Normal : Normal;
};

VertexOutput VS_CubeMapCross(VertexIN_CubeMapCross In)
{
    VertexOutput Out;
    Out.LocalDirection = In.Normal;
    Out.Position = mul(mul(float4(In.Position, 1.0), ModelMatrix), ViewProjMatrix);
    return Out;
}

float4 PS_CubeMapCross(VertexOutput In) : SV_Target
{
    float3 Color = CubeEnvironment.SampleLevel(LinearSampler, In.LocalDirection, MipLevel).xyz;

    return float4(ToneMapping(Color * Exposure), 1.0);
}

//-------------------------------------------------------
// Generate Prefiltered map
//-------------------------------------------------------

// VS is same as VS_SkyCube

float3 PrefilterEnvMap(uint2 Random, float Roughness, float3 R)
{
    float3 FilteredColor = 0;
    float Weight = 0;

    uint CubeSize = 1 << (MaxMipLevel - 1);
    const float SolidAngleTexel = 4 * PI / (6 * CubeSize * CubeSize);

    const uint NumSamples = Roughness < 0.1 ? 32 : 64;
    for (uint i = 0; i < NumSamples; i++)
    {
        float2 E = Hammersley(i, NumSamples, Random);
        float3 H = TangentToWorld(ImportanceSampleGGX(E, Pow4(Roughness)).xyz, R);
        float3 L = 2 * dot(R, H) * H - R;

        float NoL = saturate(dot(R, L));
        float NoH = saturate(dot(R, H));
        if (NoL > 0)
        {
			//https://placeholderart.wordpress.com/2015/07/28/implementation-notes-runtime-environment-map-filtering-for-image-based-lighting/
			//float PDF = D_GGX( Pow4(Roughness), NoH ) * NoH / (4 * VoH);  //NoH == VoH
            float PDF = D_GGX(Pow4(Roughness), NoH) * 0.25;
            float SolidAngleSample = 1.0 / (NumSamples * PDF);
            float MipBias = 1.0;
            float Mip = clamp(0.5 * log2(SolidAngleSample / SolidAngleTexel) + MipBias, 0, MaxMipLevel - 1);

            FilteredColor += CubeEnvironment.SampleLevel(LinearSampler, L, Mip).rgb * NoL;
            Weight += NoL;
        }
    }

    return FilteredColor / max(Weight, 0.001);
}

float4 PS_GenPrefiltered(VertexOutput In) : SV_Target
{
    int2 PixelPos = int2(In.Position.xy);
    uint2 Random = Rand3DPCG16(uint3(PixelPos, In.Position.x * 1024)).xy;

    float3 R = normalize(In.LocalDirection);
	//float Roughness = MipLevel / (MaxMipLevel - 1.0);
    float Roughness = ComputeReflectionCaptureRoughnessFromMip(MipLevel, MaxMipLevel - 1.0);
    float3 Prefiltered = PrefilterEnvMap(Random, Roughness, R);
    return float4(Prefiltered, 1.0);
}

float2 PS_PreIntegrateBRDF(in VertexOutput_Texture2D In) : SV_Target0
{
    int2 PixelPos = int2(In.Pos.xy);
    //uint2 Random = Rand3DPCG16(uint3(PixelPos, In.Pos.x * 128)).xy;

    float Roughness = In.Tex.y;
    float NoV = In.Tex.x;
    
    return IntegrateBRDF(uint2(0,0),Roughness, NoV);
}