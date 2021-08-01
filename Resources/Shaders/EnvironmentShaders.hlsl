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
    float3 Padding;
    float3 Coeffs[16];
};

TextureCube CubeEnvironment : register(t0);

float4 PS_SkyCube(VertexOutput In) : SV_Target
{
	// Local Direction don't need to normalized
    float3 Color = CubeEnvironment.Sample(LinearSampler, In.LocalDirection).xyz;

    return float4(ToneMapping(Color * Exposure), 1.0);
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