#pragma pack_matrix(row_major)

#include "ShaderUtils.hlsl"
#include "PixelPacking_Velocity.hlsli"

Texture2D<float> DepthBuffer : register(t0);
RWTexture2D<packed_velocity_t> VelocityBuffer : register(u0);

cbuffer CBuffer : register(b0)
{
    matrix CurToPrevForm;
}

[numthreads(8,8,1)]
void cs_main(uint3 DTid: SV_DispatchThreadID)
{
    uint2 st = DTid.xy;
    float2 CurPixel = st + 0.5;
    float Depth = DepthBuffer[ st];
    float4 HPos = float4(CurPixel, Depth, 1.0f);
    float4 PrevHPos = mul(HPos, CurToPrevForm);
    
    PrevHPos.xyz /= PrevHPos.w;
    VelocityBuffer[st] = PackVelocity(PrevHPos.xyz - float3(CurPixel,Depth));

}