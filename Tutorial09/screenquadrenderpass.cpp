#include "screenquadrenderpass.h"
#include "Model.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "StringUnit.h"
#include "Geometry.h"

ScreenQuadRenderPass::ScreenQuadRenderPass()
{

}

ScreenQuadRenderPass::~ScreenQuadRenderPass()
{

}

void ScreenQuadRenderPass::Init(const std::wstring& ShaderFile, int Width, int Height)
{
	m_GameWndSize = { Width,Height };
	SetupRootSignature();
	SetupPipelineState(ShaderFile);
}

void ScreenQuadRenderPass::Render(FCommandContext& CommandContext, const std::function<void(FCommandContext& CommandContext)>& BeforeDrawParam,
	const std::function<void(FCommandContext& CommandContext)>& AfterDrawParam)
{
	CommandContext.SetRootSignature(m_ScreenQuadSignature);
	CommandContext.SetViewportAndScissor(0, 0, m_GameWndSize.x, m_GameWndSize.y);

	RenderWindow& renderWindow = RenderWindow::Get();
	FColorBuffer& BackBuffer = renderWindow.GetBackBuffer();
	// Indicate that the back buffer will be used as a render target.
	CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET,true);
	CommandContext.SetRenderTargets(1, &BackBuffer.GetRTV());

	// Record commands.
	CommandContext.ClearColor(BackBuffer);
	CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CommandContext.SetPipelineState(m_ScreenQuadRenderState->GetPipelineState());

	if (BeforeDrawParam)
	{
		BeforeDrawParam(CommandContext);
	}
	CommandContext.Draw(3);

	if (AfterDrawParam)
	{
		AfterDrawParam(CommandContext);
	}

	CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_PRESENT);
}

void ScreenQuadRenderPass::SetupRootSignature()
{
	FSamplerDesc DefaultSamplerDesc;
	m_ScreenQuadSignature.Reset(3, 1);
	m_ScreenQuadSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_ScreenQuadSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_ScreenQuadSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_ScreenQuadSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_ScreenQuadSignature.Finalize(L"ScreenQuadSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void ScreenQuadRenderPass::SetupPipelineState(const std::wstring& ShaderFile)
{
	std::shared_ptr<FShader> Shader = FShaderMgr::Get().CreateShader(core::usc2_u8(ShaderFile), ShaderFile);
	m_ScreenQuadRenderState = std::make_shared<FRenderPipelineInfo>(Shader);
	m_ScreenQuadRenderState->SetupRenderTargetFormat(1, &RenderWindow::Get().GetColorFormat(), DXGI_FORMAT_UNKNOWN);
	m_ScreenQuadRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerTwoSided);
	m_ScreenQuadRenderState->SetBlendState(FGraphicsPipelineState::BlendDisable);
	m_ScreenQuadRenderState->SetDepthStencilState(FGraphicsPipelineState::DepthStateDisabled);

	m_ScreenQuadRenderState->SetupPipeline(m_ScreenQuadSignature);
	m_ScreenQuadRenderState->PipelineFinalize();

}
