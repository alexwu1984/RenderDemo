#include "BlurFilter.h"
#include "Shader.h"
#include "CommandContext.h"

FBlurFilter::FBlurFilter(std::shared_ptr<FShader> BlurHorizontalShader, std::shared_ptr<FShader> BlurVerticalShader, uint32_t width, uint32_t height)
	:m_BlurHorizontalShader(BlurHorizontalShader)
	,m_BlurVerticalShader(BlurVerticalShader)
	,m_Width(width)
	,m_Height(height)
{
	BuildResource();
}

FBlurFilter::~FBlurFilter()
{

}

void FBlurFilter::Execute(FD3D12Resource& texInput, FComputeContext& CommandContext, int BlurCount)
{

	CommandContext.CopyBuffer(m_BlurBuf0, texInput);

	for (int i = 0; i < BlurCount; ++i)
	{
		// Horizontal Blur pass.
		CommandContext.TransitionResource(m_BlurBuf0, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		CommandContext.TransitionResource(m_BlurBuf1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		CommandContext.SetRootSignature(m_CSSignature);
		CommandContext.SetPipelineState(m_BlurHorizontalPSO);

		CommandContext.SetDynamicDescriptor(2, 0, m_SettinglCpuHandle);
		CommandContext.SetDynamicDescriptor(0, 0, m_BlurBuf0.GetSRV());
		CommandContext.SetDynamicDescriptor(1, 0, m_BlurBuf1.GetUAV());


		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(m_Width / 256.0f);
		CommandContext.Dispatch(numGroupsX, m_Height, 1);

		//
		// Vertical Blur pass.
		//
		CommandContext.TransitionResource(m_BlurBuf1, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		CommandContext.TransitionResource(m_BlurBuf0, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		CommandContext.SetPipelineState(m_BlurVerticalPSO);

		CommandContext.SetDynamicDescriptor(2, 0, m_SettinglCpuHandle);
		CommandContext.SetDynamicDescriptor(0, 0, m_BlurBuf1.GetSRV());
		CommandContext.SetDynamicDescriptor(1, 0, m_BlurBuf0.GetUAV());

		// How many groups do we need to dispatch to cover a column of pixels, where each
		// group covers 256 pixels  (the 256 is defined in the ComputeShader).
		UINT numGroupsY = (UINT)ceilf(m_Height / 256.0f);
		CommandContext.Dispatch(m_Height, numGroupsY, 1);
	}


}

const D3D12_CPU_DESCRIPTOR_HANDLE& FBlurFilter::GetSRV()
{
	return m_BlurBuf0.GetSRV();
}

FColorBuffer& FBlurFilter::GetBuffer()
{
	return m_BlurBuf0;
}

void FBlurFilter::BuildResource()
{
	m_CSSignature.Reset(3, 0);
	m_CSSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	m_CSSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	m_CSSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
	m_CSSignature.Finalize(L"Blur Compute Shader RootSignature");

	m_BlurHorizontalPSO.SetRootSignature(m_CSSignature);
	m_BlurHorizontalPSO.SetComputeShader(CD3DX12_SHADER_BYTECODE(m_BlurHorizontalShader->GetComputeShader().Get()));
	m_BlurHorizontalPSO.Finalize();

	m_BlurVerticalPSO.SetRootSignature(m_CSSignature);
	m_BlurVerticalPSO.SetComputeShader(CD3DX12_SHADER_BYTECODE(m_BlurVerticalShader->GetComputeShader().Get()));
	m_BlurVerticalPSO.Finalize();

	auto weights = CalcGaussWeights(2.5f);
	int blurRadius = (int)weights.size() / 2;
	m_ConstSetting.gBlurRadius = blurRadius;
	for (size_t i = 0; i < weights.size(); i++)
	{
		(&m_ConstSetting.w0)[i] = weights[i];
	}
	m_SettinglConstBuf.CreateUpload(L"ConstSetting", sizeof(m_ConstSetting));
	m_SettinglCpuHandle = m_SettinglConstBuf.CreateConstantBufferView(0, sizeof(m_ConstSetting));
	memcpy(m_SettinglConstBuf.Map(), &m_ConstSetting, sizeof(m_ConstSetting));

	m_BlurBuf0.Create(L"BlurBuf0", m_Width, m_Height, 1, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_BlurBuf1.Create(L"BlurBuf1", m_Width, m_Height, 1, DXGI_FORMAT_R8G8B8A8_UNORM);
}

std::vector<float> FBlurFilter::CalcGaussWeights(float sigma)
{
	float twoSigma2 = 2.0f * sigma * sigma;

	// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
	// For example, for sigma = 3, the width of the bell curve is 
	int blurRadius = (int)ceil(2.0f * sigma);

	Assert(blurRadius <= MaxBlurRadius);

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);

	float weightSum = 0.0f;

	for (int i = -blurRadius; i <= blurRadius; ++i)
	{
		float x = (float)i;

		weights[i + blurRadius] = expf(-x * x / twoSigma2);

		weightSum += weights[i + blurRadius];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for (int i = 0; i < weights.size(); ++i)
	{
		weights[i] /= weightSum;
	}

	return weights;
}

