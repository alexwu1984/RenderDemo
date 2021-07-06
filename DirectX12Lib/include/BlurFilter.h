#pragma once
#include "Common.h"
#include "MathLib.h"
#include "GpuBuffer.h"
#include "ColorBuffer.h"
#include "PipelineState.h"
#include "RootSignature.h"

class FShader;
class FComputeContext;

class FBlurFilter
{
public:
	FBlurFilter(std::shared_ptr<FShader> BlurHorizontalShader, std::shared_ptr<FShader> BlurVerticalShader, uint32_t width, uint32_t height);
	~FBlurFilter();

	void Execute(FD3D12Resource& texInput, FComputeContext& CommandContext,int BlurCount);
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV();
	FColorBuffer& GetBuffer();


private:
	void BuildResource();

	std::vector<float> CalcGaussWeights(float sigma);

	const int MaxBlurRadius = 5;
	struct cbSettings 
	{
		// We cannot have an array entry in a constant buffer that gets mapped onto
		// root constants, so list each element.  

		int gBlurRadius;

		// Support up to 11 blur weights.
		float w0;
		float w1;
		float w2;
		float w3;
		float w4;
		float w5;
		float w6;
		float w7;
		float w8;
		float w9;
		float w10;
	};
	cbSettings m_ConstSetting;
	FConstBuffer m_SettinglConstBuf;
	D3D12_CPU_DESCRIPTOR_HANDLE m_SettinglCpuHandle;
	FColorBuffer m_BlurBuf0;
	FColorBuffer m_BlurBuf1;

	FRootSignature m_CSSignature;
	FComputePipelineState m_BlurHorizontalPSO;
	FComputePipelineState m_BlurVerticalPSO;

	std::shared_ptr<FShader> m_BlurHorizontalShader;
	std::shared_ptr<FShader> m_BlurVerticalShader;
	uint32_t m_Width=0;
	uint32_t m_Height=0;

	std::vector<float> m_GaussWeights;
};