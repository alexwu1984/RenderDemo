#pragma once
#include "Common.h"
#include "PipelineState.h"

class FShader;
class FRenderPipelineInfo
{
public:
	FRenderPipelineInfo(std::shared_ptr< FShader> shader);
	~FRenderPipelineInfo() {}
	void SetupPipeline(FRootSignature& rootSignature, std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements);
	void SetupPipeline(FRootSignature& rootSignature);
	void SetRasterizerState(const D3D12_RASTERIZER_DESC& RasterizerDesc);
	void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc);
	void SetBlendState(const D3D12_BLEND_DESC& BlendDesc);
	void SetupRenderTargetFormat(uint32_t NumRTVs, const DXGI_FORMAT* RTVFormats, DXGI_FORMAT DSVFormat);
	void PipelineFinalize();
	FGraphicsPipelineState& GetPipelineState();

private:

	std::shared_ptr< FShader> m_Shader;
	FGraphicsPipelineState  m_PipelineState;
};