#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"
#include "Texture.h"
#include "GpuBuffer.h"

class FCommandContext;
class RenderPipelineInfo;
class FDepthBuffer;

class HBAOPass
{
public:
	HBAOPass();
	~HBAOPass();

	void Init(int Width, int Height);
	void Render(FCommandContext& CommandContext, FDepthBuffer& Depth);
	FColorBuffer& GetAOBuffer();

private:
	void SetupRootSignature();
	void SetupPipelineState();

	std::vector<Vector4f> GenerateNoise();

	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_RootSignature;
	Vector2<int> m_GameWndSize;
	FTexture m_NoiseTexture;
	FColorBuffer m_AOBuffer;

	struct HBAOPassInfo
	{
		float Near=0.1;
		float Far=100.f;
		float fov = 45;
		float pad;
		Vector2f FocalLen;
		float AOStrength = 1.9;
		float WindowWidth = 0;
		float WindowHeight = 0;
		float MaxRadiusPixels = 50.0;
		float R = 0.3;
		float R2 = 0.3 * 0.3;
		float NegInvR2 = -1.0 / (0.3 * 0.3);
		float TanBias = std::tan(30.0 * MATH_PI / 180.0);
		Vector2f pad1;
	};
	HBAOPassInfo m_PassInfo;
	FConstBuffer m_AOPassConstBuf;
	D3D12_CPU_DESCRIPTOR_HANDLE m_AOPassCpuHandle;

};