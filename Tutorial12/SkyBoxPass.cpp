#include "SkyBoxPass.h"
#include "SkyBox.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "StringUnit.h"
#include "Shader.h"
#include "CubeBuffer.h"

SkyBoxPass::SkyBoxPass()
{

}

SkyBoxPass::~SkyBoxPass()
{

}

void SkyBoxPass::Init(std::shared_ptr< FSkyBox> skyBox, int32_t width, int height)
{
	m_Size = { width,height };
	m_SkyBox = skyBox;
	SetupRootSignature();
	SetupPipelineState(L"../Resources/Shaders/Tutorial12/EnvironmentShaders.hlsl", "VS_SkyCube", "PS_SkyCube");
}

void SkyBoxPass::Render(FCommandContext& GfxContext, FCamera& MainCamera,FCubeBuffer& CubeBuffer)
{
	GfxContext.SetRootSignature(m_SkySignature);
	GfxContext.SetPipelineState(m_RenderState->GetPipelineState());
	GfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GfxContext.SetViewportAndScissor(0, 0, m_Size.x, m_Size.y);

	RenderWindow& renderWindow = RenderWindow::Get();
	FColorBuffer& BackBuffer = renderWindow.GetBackBuffer();
	FDepthBuffer& DepthBuffer = renderWindow.GetDepthBuffer();

	// Indicate that the back buffer will be used as a render target.
	GfxContext.TransitionResource(CubeBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	GfxContext.TransitionResource(DepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);

	GfxContext.SetRenderTargets(1, &BackBuffer.GetRTV());
	GfxContext.SetRenderTargets(1, &BackBuffer.GetRTV(), DepthBuffer.GetDSV());

	GfxContext.ClearColor(BackBuffer);
	GfxContext.ClearDepth(DepthBuffer);

	g_EVSConstants.ModelMatrix = FMatrix::TranslateMatrix(MainCamera.GetPosition()); // move with camera
	g_EVSConstants.ViewProjMatrix = MainCamera.GetViewMatrix() * MainCamera.GetProjectionMatrix();
	GfxContext.SetDynamicConstantBufferView(0, sizeof(g_EVSConstants), &g_EVSConstants);

	g_EPSConstants.Exposure = 1.0;
	GfxContext.SetDynamicConstantBufferView(1, sizeof(g_EPSConstants), &g_EPSConstants);

	GfxContext.SetDynamicDescriptor(2, 0, CubeBuffer.GetCubeSRV());

	m_SkyBox->Draw(GfxContext);
}

void SkyBoxPass::SetupRootSignature()
{
	FSamplerDesc PointSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	m_SkySignature.Reset(3, 1);
	m_SkySignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_SkySignature[1].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_SkySignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	m_SkySignature.InitStaticSampler(0, PointSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_SkySignature.Finalize(L"SkyBoxPass", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void SkyBoxPass::SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint)
{
	std::shared_ptr<FShader> Shader = FShaderMgr::Get().CreateShader(core::usc2_u8(ShaderFile), entryVSPoint, entryPSPoint, ShaderFile);
	m_RenderState = std::make_shared<RenderPipelineInfo>(Shader);

	m_RenderState->SetupRenderTargetFormat(1, &RenderWindow::Get().GetColorFormat(), RenderWindow::Get().GetDepthFormat());
	m_RenderState->SetRasterizerState(FPipelineState::RasterizerTwoSided);
	m_RenderState->SetDepthStencilState(FPipelineState::DepthStateDisabled);
	m_RenderState->SetBlendState(FPipelineState::BlendDisable);

	std::vector<D3D12_INPUT_ELEMENT_DESC> SkyBoxLayout;
	m_SkyBox->GetMeshLayout(SkyBoxLayout);

	m_RenderState->SetupPipeline(m_SkySignature, SkyBoxLayout);
	m_RenderState->PipelineFinalize();
}
