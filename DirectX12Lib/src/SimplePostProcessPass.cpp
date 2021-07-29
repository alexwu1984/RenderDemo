#include "SimplePostProcessPass.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "StringUnit.h"
#include "Shader.h"

SimplePostProcessPass::SimplePostProcessPass()
{

}

SimplePostProcessPass::~SimplePostProcessPass()
{

}

void SimplePostProcessPass::Init(const std::wstring& ShaderFile, int Width, int Height)
{
	m_GameWndSize = { Width,Height };
	SetupRootSignature();
	SetupPipelineState(ShaderFile);
}

void SimplePostProcessPass::Init(const std::wstring& ShaderFile, const std::string& vs, const std::string& ps,DXGI_FORMAT format)
{
	m_RenderTargetFormat = format;
	SetupRootSignature();
	SetupPipelineState(ShaderFile, vs,ps);
}

void SimplePostProcessPass::Render(FCommandContext& CommandContext, const std::function<void(FCommandContext& CommandContext)>& BeforeDrawParam, 
	const std::function<void(FCommandContext& CommandContext)>& AfterDrawParam)
{
	// Set necessary state.
	CommandContext.SetRootSignature(m_RootSignature);
	CommandContext.SetViewportAndScissor(m_Pos.x, m_Pos.y, m_GameWndSize.x, m_GameWndSize.y);

	// Indicate that the back buffer will be used as a render target.
	CommandContext.TransitionResource(m_PostRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET,true);

	const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] = { m_PostRenderTarget.GetRTV() };
	CommandContext.SetRenderTargets(1, RTVs);

	// Record commands.
	CommandContext.ClearColor(m_PostRenderTarget);
	CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandContext.SetPipelineState(m_RenderState->GetPipelineState());

	if (BeforeDrawParam)
	{
		BeforeDrawParam(CommandContext);
	}

	CommandContext.Draw(3);

	if (AfterDrawParam)
	{
		AfterDrawParam(CommandContext);
	}
}

void SimplePostProcessPass::Render(FColorBuffer& RenderTarget, FCommandContext& CommandContext, const std::function<void(FCommandContext& CommandContext)>& BeforeDrawParam, 
	const std::function<void(FCommandContext& CommandContext)>& AfterDrawParam)
{
	CommandContext.SetRootSignature(m_RootSignature);
	CommandContext.SetViewportAndScissor(m_Pos.x, m_Pos.y, m_GameWndSize.x, m_GameWndSize.y);

	// Indicate that the back buffer will be used as a render target.
	CommandContext.TransitionResource(RenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

	const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] = { RenderTarget.GetRTV() };
	CommandContext.SetRenderTargets(1, RTVs);

	// Record commands.
	CommandContext.ClearColor(RenderTarget);
	CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandContext.SetPipelineState(m_RenderState->GetPipelineState());

	if (BeforeDrawParam)
	{
		BeforeDrawParam(CommandContext);
	}

	CommandContext.Draw(3);

	if (AfterDrawParam)
	{
		AfterDrawParam(CommandContext);
	}
}

FColorBuffer& SimplePostProcessPass::GetResult()
{
	return m_PostRenderTarget;
}

int32_t SimplePostProcessPass::GetCBVRootIndex() const
{
	return m_CBVRootIndex;
}

int32_t SimplePostProcessPass::GetSRVRootIndex() const
{
	return m_SRVRootIndex;
}

void SimplePostProcessPass::SetupRootSignature()
{
	FSamplerDesc DefaultSamplerDesc;
	m_RootSignature.Reset(2, 1);
	m_RootSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX);
	m_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature.Finalize(L"SimplePostProcessPass", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void SimplePostProcessPass::SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint)
{
	std::shared_ptr<FShader> Shader = FShaderMgr::Get().CreateShader(core::usc2_u8(ShaderFile), entryVSPoint, entryPSPoint, ShaderFile);
	m_RenderState = std::make_shared<RenderPipelineInfo>(Shader);
	m_RenderState->SetupRenderTargetFormat(1, &m_RenderTargetFormat, DXGI_FORMAT_UNKNOWN);
	m_RenderState->SetRasterizerState(FPipelineState::RasterizerTwoSided);
	m_RenderState->SetDepthStencilState(FPipelineState::DepthStateDisabled);
	m_RenderState->SetBlendState(FPipelineState::BlendDisable);

	m_RenderState->SetupPipeline(m_RootSignature);
	m_RenderState->PipelineFinalize();

	if (m_GameWndSize.x > 0 && m_GameWndSize.y > 0)
	{
		m_PostRenderTarget.Create(L"PostRenderTarget", m_GameWndSize.x, m_GameWndSize.y, 1, m_RenderTargetFormat);
	}
	
}

void SimplePostProcessPass::SetViewportAndScissor(int32_t x, int32_t y, int32_t w, int32_t h)
{
	m_GameWndSize = { w,h };
	m_Pos = { x,y };
}