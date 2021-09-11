#pragma pack_matrix(row_major)


Texture2D ShadowTexture : register(t1);
SamplerState LinearSampler : register(s0);
SamplerComparisonState GsamShadow : register(s1);
SamplerState ShadowSampler : register(s2);

#define FILTER_SIZE 8
#define GS2 ((FILTER_SIZE-1)/2)
#define PCF_SAMPLE_COUNT ((FILTER_SIZE-1)*(FILTER_SIZE-1))
#define USE_RECEIVER_PLANE_DEPTH_BIAS 1


float2 ComputeReceiverPlaneDepthBias(float3 texCoordDX, float3 texCoordDY)
{
    float2 biasUV;
    biasUV.x = texCoordDY.y * texCoordDX.z - texCoordDX.y * texCoordDY.z;
    biasUV.y = texCoordDX.x * texCoordDY.z - texCoordDY.x * texCoordDX.z;
    biasUV *= 1.0f / ((texCoordDX.x * texCoordDY.y) - (texCoordDX.y * texCoordDY.x));
    return biasUV;
}

float DoSampleCmp(float2 ReceiverPlaneDepthBias, float2 TexelSize, float2 SubTexelCoord, float CurrentDepth, int2 TexelOffset)
{
#if USE_RECEIVER_PLANE_DEPTH_BIAS
    CurrentDepth += dot(TexelOffset * TexelSize, ReceiverPlaneDepthBias);
#endif
    return ShadowTexture.SampleCmpLevelZero(GsamShadow, SubTexelCoord, CurrentDepth, TexelOffset);
}

float SampleFixedSizePCF(float3 ShadowPos, float3 LightDirection, float3 Normal)
{
    float NumSlices;
    float2 ShadowMapSize;
    ShadowTexture.GetDimensions(0, ShadowMapSize.x, ShadowMapSize.y, NumSlices);
    float2 TexelSize = 1.0 / ShadowMapSize;

    float2 tc = ShadowPos.xy;
    float2 stc = ShadowMapSize * tc + float2(0.5, 0.5);
    float2 tcs = floor(stc);
	
    float2 fc = stc - tcs;
    tc = tc - fc * TexelSize; // x in "Fast Conventional Shadow Filtering"

    float2 pwAB = 2.0 - fc;
    float2 tcAB = TexelSize / pwAB;
    float2 pwM = 2.0;
    float2 tcM = 0.5 * TexelSize;
    float2 pwGH = 1.0 + fc;
    float2 tcGH = TexelSize * fc / pwGH;

    float CurrentDepth = ShadowPos.z;
    float2 ReceiverPlaneDepthBias = float2(0.0, 0.0);

#if USE_RECEIVER_PLANE_DEPTH_BIAS
    float3 ShadowPosDX = ddx_fine(ShadowPos);
    float3 ShadowPosDY = ddy_fine(ShadowPos);
    ReceiverPlaneDepthBias = ComputeReceiverPlaneDepthBias(ShadowPosDX, ShadowPosDY);
    float FractionalSamplingError = dot(TexelSize, abs(ReceiverPlaneDepthBias));
    CurrentDepth -= min(FractionalSamplingError, 0.006f);
#endif

    float bias = clamp(0.002 + (saturate(dot(LightDirection, Normal))) * 0.01, 0.0, 0.003);
    bias = (1 - saturate(dot(LightDirection, Normal))) * 0.02;
	//CurrentDepth -= bias;

    float s = 0.0;
    s += pwAB.x * pwAB.y * DoSampleCmp(ReceiverPlaneDepthBias, TexelSize, tc + tcAB, CurrentDepth, int2(-GS2, -GS2)); //top left
    s += pwGH.x * pwAB.y * DoSampleCmp(ReceiverPlaneDepthBias, TexelSize, tc + float2(tcGH.x, tcAB.y), CurrentDepth, int2(GS2, -GS2)); //top right
    s += pwAB.x * pwGH.y * DoSampleCmp(ReceiverPlaneDepthBias, TexelSize, tc + float2(tcAB.x, tcGH.y), CurrentDepth, int2(-GS2, GS2)); //bottom left
    s += pwGH.x * pwGH.y * DoSampleCmp(ReceiverPlaneDepthBias, TexelSize, tc + tcGH, CurrentDepth, int2(GS2, GS2)); //bottom right
	
    for (int col = 2 - GS2; col <= GS2 - 2; col += 2)
    {
        s += pwM * pwAB.y * DoSampleCmp(ReceiverPlaneDepthBias, TexelSize, tc + float2(tcM.x, tcAB.y), CurrentDepth, int2(col, -GS2)); //top row
        s += pwM * pwGH.y * DoSampleCmp(ReceiverPlaneDepthBias, TexelSize, tc + float2(tcM.x, tcGH.y), CurrentDepth, int2(col, GS2)); //bottom row
        s += pwAB.x * pwM * DoSampleCmp(ReceiverPlaneDepthBias, TexelSize, tc + float2(tcAB.x, tcM.y), CurrentDepth, int2(-GS2, col)); //left col
        s += pwGH.x * pwM * DoSampleCmp(ReceiverPlaneDepthBias, TexelSize, tc + float2(tcGH.x, tcM.y), CurrentDepth, int2(GS2, col)); //right col
    }

	// middle
    for (int row = 2 - GS2; row <= GS2 - 2; row += 2)
    {
        for (int col = 2 - GS2; col <= GS2 - 2; col += 2)
        {
            s += pwM * pwM * DoSampleCmp(ReceiverPlaneDepthBias, TexelSize, tc + tcM, CurrentDepth, int2(col, row));
        }
    }

    return s / PCF_SAMPLE_COUNT;
}

float ComputeShadow(float3 ShadowCoord, float3 Normal, float3 LightDirection)
{
#if FILTER_SIZE == 2
    return CalcShadowFactor(ShadowTexture, ShadowCoord);
#else
    return SampleFixedSizePCF(ShadowCoord, -LightDirection, Normal);
#endif
}

float CalcShadowFactor(Texture2D shadowMap, float3 shadowPosH)
{

    // Depth in NDC space.
    float depth = shadowPosH.z;

    uint width, height, numMips;
    shadowMap.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float) width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(GsamShadow,
            shadowPosH.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
}

float Linstep(float a,float b,float v)
{
    return saturate((v - a) / (b - a));
}

float ReduceLightBleeding(float pMax, float amount)
{
    return Linstep(amount, 1.0f, pMax);
}

float ChebyshevUpperBound(float2 Moments,float t)
{
    float Variance = Moments.y - Moments.x * Moments.x;
    float MinVariance = 0.0000001;
    Variance = max(Variance, MinVariance);
    
    float d = t - Moments.x;
    float pMax = Variance / (Variance + d * d);
    
    float lightBleedingReduction = 0.0;
    pMax = ReduceLightBleeding(pMax, lightBleedingReduction);
    
    return (t -0.06 <= Moments.x ? 1.0 : pMax);
}

float ComputeShadow_VSM(float3 ShadowCoord, float3 Normal, float3 LightDirection)
{
    float2 Moments = ShadowTexture.Sample(ShadowSampler, ShadowCoord.xy).xy;
    
    float bias = max(0.01 * (1.0 - dot(Normal, -LightDirection)), 0.001);
    
    return ChebyshevUpperBound(Moments, saturate(ShadowCoord.z) - bias);

}