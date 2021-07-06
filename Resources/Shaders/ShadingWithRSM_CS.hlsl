#pragma pack_matrix(row_major)

#define LOCAL_GROUP_SIZE 16
#define VPL_NUM 32


struct RSMBasePassInfo
{
    float4 VPLsSampleCoordsAndWeights[VPL_NUM];
    float  MaxSampleRadius;
    float RSMSize;
    int VPLNum;
    float3 LightDirInViewSpace;
    float4x4 LightVP;
    float4x4 InverseCameraView;
};

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D PositionTexture : register(t2);
Texture2D RSMFluxTexture : register(t3);
Texture2D RSMNormalTexture : register(t4);
Texture2D RSMPositionTexture : register(t5);
RWTexture2D<float4> OutputImage : register(u0);

ConstantBuffer<RSMBasePassInfo> BasePass : register(b0);

SamplerState LinearSampler : register(s0);



float3 calcVPLIrradiance(float3 vVPLFlux, float3 vVPLNormal, float3 vVPLPos, float3 vFragPos, float3 vFragNormal, float vWeight)
{
    float3 VPL2Frag = normalize(vFragPos - vVPLPos);
	return vVPLFlux * max(dot(vVPLNormal, VPL2Frag), 0) * max(dot(vFragNormal, -VPL2Frag), 0) * vWeight;
}

[numthreads(16, 16, 1)]
void cs_main(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    if (BasePass.VPLNum != VPL_NUM)
		return;

    float2 FragPos = float2(DispatchThreadID.xy);
    float3 FragViewNormal = normalize(NormalTexture.Load(int3(FragPos, 0), int2(0, 0)).xyz);
	
    float3 FragAlbedo = AlbedoTexture.Load(int3(FragPos, 0), int2(0, 0)).xyz;
    float3 FragViewPos = PositionTexture.Load(int3(FragPos, 0), int2(0, 0)).xyz;
    float4 FragPosInLightSpace = mul(float4(FragViewPos, 1), BasePass.InverseCameraView);
    FragPosInLightSpace = mul(FragPosInLightSpace, BasePass.LightVP);
	FragPosInLightSpace /= FragPosInLightSpace.w;
	float2 FragNDCPos4Light = (FragPosInLightSpace.xy + 1) / 2;
    float RSMTexelSize = 1.0 / BasePass.RSMSize;

    float3 DirectIllumination = FragAlbedo * max(dot(-BasePass.LightDirInViewSpace.xyz, FragViewNormal), 0);

    float3 IndirectIllumination = float3(0,0,0);
    for (int i = 0; i < BasePass.VPLNum; ++i)
	{
        float3 VPLSampleCoordAndWeight = BasePass.VPLsSampleCoordsAndWeights[i].xyz;
        float2 VPLSamplePos = FragNDCPos4Light + BasePass.MaxSampleRadius * VPLSampleCoordAndWeight.xy * RSMTexelSize;
        float3 VPLFlux = RSMFluxTexture.SampleLevel(LinearSampler, VPLSamplePos,0).xyz;
        float3 VPLNormalInViewSpace = normalize(RSMNormalTexture.SampleLevel(LinearSampler, VPLSamplePos, 0).xyz);
        float3 VPLPositionInViewSpace = RSMPositionTexture.SampleLevel(LinearSampler, VPLSamplePos, 0).xyz;

		IndirectIllumination += calcVPLIrradiance(VPLFlux, VPLNormalInViewSpace, VPLPositionInViewSpace, FragViewPos, FragViewNormal, VPLSampleCoordAndWeight.z);
	}
	IndirectIllumination *= FragAlbedo;

    float3 Result = DirectIllumination * 0.6 + IndirectIllumination;

    OutputImage[FragPos] = float4(Result, 1.0);
}