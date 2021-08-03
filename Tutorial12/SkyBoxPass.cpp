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

void SkyBoxPass::Init(std::shared_ptr< FModel> skyBox, int32_t width, int height, const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint)
{
	m_Size = { width,height };
	m_SkyBox = skyBox;
	SetupRootSignature();
	SetupPipelineState(ShaderFile, entryVSPoint, entryPSPoint);
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

void SkyBoxPass::ShowCubeMapDebugView(FCommandContext& GfxContext, FCubeBuffer& CubeBuffer, float Exposure, int MipLevel)
{
	// Set necessary state.
	GfxContext.SetRootSignature(m_SkySignature);
	GfxContext.SetPipelineState(m_RenderState->GetPipelineState());
	GfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	uint32_t Size = std::min(m_Size.x, m_Size.y);
	//Size = std::min(Size, CubeBuffer.GetWidth());
	GfxContext.SetViewportAndScissor((m_Size.x - Size) / 2, (m_Size.y - Size) / 2, Size, Size);

	RenderWindow& renderWindow = RenderWindow::Get();
	FColorBuffer& BackBuffer = renderWindow.GetBackBuffer();

	// Indicate that the back buffer will be used as a render target.
	GfxContext.TransitionResource(CubeBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

	GfxContext.SetRenderTargets(1, &BackBuffer.GetRTV());

	GfxContext.ClearColor(BackBuffer);

	g_EVSConstants.ModelMatrix = FMatrix(); // identity
	g_EVSConstants.ViewProjMatrix = FMatrix::MatrixOrthoLH(1.f, 1.f, -1.f, 1.f);
	GfxContext.SetDynamicConstantBufferView(0, sizeof(g_EVSConstants), &g_EVSConstants);

	g_EPSConstants.Exposure = Exposure;
	g_EPSConstants.MipLevel = MipLevel;
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
	std::shared_ptr<FShader> Shader = FShaderMgr::Get().CreateShaderDirect(ShaderFile, entryVSPoint, entryPSPoint);
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
