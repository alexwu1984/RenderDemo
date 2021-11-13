#include "RenderPipelineInfo.h"
#include "Shader.h"

FRenderPipelineInfo::FRenderPipelineInfo(std::shared_ptr< FShader> shader)
	:m_Shader(shader)
{
	m_PipelineState.SetRasterizerState(FPipelineState::RasterizerDefault);
	m_PipelineState.SetBlendState(FPipelineState::BlendTraditional);
	m_PipelineState.SetDepthStencilState(FPipelineState::DepthStateReadWrite);
	m_PipelineState.SetVertexShader(CD3DX12_SHADER_BYTECODE(m_Shader->GetVertexShader().Get()));
	m_PipelineState.SetPixelShader(CD3DX12_SHADER_BYTECODE(m_Shader->GetPixelShader().Get()));
}

void FRenderPipelineInfo::SetupPipeline(FRootSignature& rootSignature, std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements)
{
	Assert(inputElements.size() > 0);
	m_PipelineState.SetRootSignature(rootSignature);
	
	m_PipelineState.SetInputLayout((UINT)inputElements.size(), &inputElements[0]);
	m_PipelineState.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	
}

void FRenderPipelineInfo::SetupPipeline(FRootSignature& rootSignature)
{
	m_PipelineState.SetRootSignature(rootSignature);
	m_PipelineState.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
}

void FRenderPipelineInfo::SetRasterizerState(const D3D12_RASTERIZER_DESC& RasterizerDesc)
{
	m_PipelineState.SetRasterizerState(RasterizerDesc);
}

void FRenderPipelineInfo::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc)
{
	m_PipelineState.SetDepthStencilState(DepthStencilDesc);
}

void FRenderPipelineInfo::SetBlendState(const D3D12_BLEND_DESC& BlendDesc)
{
	m_PipelineState.SetBlendState(BlendDesc);
}

void FRenderPipelineInfo::SetupRenderTargetFormat(uint32_t NumRTVs, const DXGI_FORMAT* RTVFormats, DXGI_FORMAT DSVFormat)
{
	m_PipelineState.SetRenderTargetFormats(NumRTVs, RTVFormats, DSVFormat);
}

void FRenderPipelineInfo::PipelineFinalize()
{
	m_PipelineState.Finalize();
}

FGraphicsPipelineState& FRenderPipelineInfo::GetPipelineState()
{
	return m_PipelineState;
}
