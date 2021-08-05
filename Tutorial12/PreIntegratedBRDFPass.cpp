#include "PreIntegratedBRDFPass.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "StringUnit.h"
#include "Shader.h"
#include "Texture.h"

PreIntegratedBRDFPass::PreIntegratedBRDFPass()
{

}

PreIntegratedBRDFPass::~PreIntegratedBRDFPass()
{

}

void PreIntegratedBRDFPass::Init()
{
	SetupRootSignature();
	SetupPipelineState(L"../Resources/Shaders/EnvironmentShaders.hlsl", "VS_ShowTexture2D", "PS_PreIntegrateBRDF");
}


void PreIntegratedBRDFPass::IntegrateBRDF(FColorBuffer& target)
{
	FCommandContext& GfxContext = FCommandContext::Begin(D3D12_COMMAND_LIST_TYPE_DIRECT, L"3D Queue");
	RenderWindow& renderWindow = RenderWindow::Get();

	// Set necessary state.
	GfxContext.SetRootSignature(m_RootSignature);
	GfxContext.SetViewportAndScissor(0,0, m_Size.x, m_Size.y);

	// Indicate that the back buffer will be used as a render target.
	GfxContext.TransitionResource(target, D3D12_RESOURCE_STATE_RENDER_TARGET,true);

	const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] = { target.GetRTV() };
	GfxContext.SetRenderTargets(1, RTVs);

	// Record commands.
	GfxContext.ClearColor(target);
	GfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GfxContext.SetPipelineState(m_RenderState->GetPipelineState());
	
	g_EVSConstants.ModelMatrix = FMatrix(); // identity
	g_EVSConstants.ViewProjMatrix = FMatrix::MatrixOrthoLH(1.f, 1.f, -1.f, 1.f);
	GfxContext.SetDynamicConstantBufferView(0, sizeof(EVSConstants), &g_EVSConstants);
	GfxContext.SetDynamicConstantBufferView(1, sizeof(EPSConstants), &g_EPSConstants);

	GfxContext.Draw(3);
	GfxContext.Finish(true);
}

void PreIntegratedBRDFPass::SetupRootSignature()
{
	FSamplerDesc PointSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	m_RootSignature.Reset(3, 1);
	m_RootSignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_RootSignature[1].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	m_RootSignature.InitStaticSampler(0, PointSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature.Finalize(L"PreIntegrateBRDFPass", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void PreIntegratedBRDFPass::SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint)
{
	std::shared_ptr<FShader> Shader = FShaderMgr::Get().CreateShaderDirect(ShaderFile, entryVSPoint, entryPSPoint);
	m_RenderState = std::make_shared<RenderPipelineInfo>(Shader);

	DXGI_FORMAT format = DXGI_FORMAT_R32G32_FLOAT;
	m_RenderState->SetupRenderTargetFormat(1, &format, DXGI_FORMAT_UNKNOWN);
	m_RenderState->SetRasterizerState(FPipelineState::RasterizerTwoSided);
	m_RenderState->SetDepthStencilState(FPipelineState::DepthStateDisabled);
	m_RenderState->SetBlendState(FPipelineState::BlendDisable);

	m_RenderState->SetupPipeline(m_RootSignature);
	m_RenderState->PipelineFinalize();
	
}
