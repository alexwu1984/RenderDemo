#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"
#include "Texture.h"
#include "GpuBuffer.h"

class FCommandContext;
class RenderPipelineInfo;
class FCamera;

class SSDOPass
{
public:
	SSDOPass();
	~SSDOPass();

	void Init(int Width, int Height);
	void Render(FCommandContext& CommandContext, FColorBuffer& PositionBuffer,FColorBuffer& NormalBuffer, FColorBuffer& AlbedoBuffer);
	FColorBuffer& GetAOBuffer();

private:
	void SetupRootSignature();
	void SetupPipelineState();

	void GenerateKernel();
	std::vector<Vector4f> GenerateNoise();

	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_RootSignature;
	Vector2<int> m_GameWndSize;
	FTexture m_NoiseTexture;
	FColorBuffer m_AOBuffer;

	struct SSDOPassInfo
	{
		Vector4f Samples[64];
		float WindowWidth;
		float WindowHeight;
		int KernelSize = 64;
		float Radius = 1.0;
		float IndirectLightScale = 5.0;
		Vector3f pad;
		FMatrix Proj;
	};
	SSDOPassInfo m_PassInfo;
	FConstBuffer m_AOPassConstBuf;
	D3D12_CPU_DESCRIPTOR_HANDLE m_AOPassCpuHandle;
};