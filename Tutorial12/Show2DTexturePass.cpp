#include "Show2DTexturePass.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "StringUnit.h"
#include "Shader.h"

Show2DTexturePass::Show2DTexturePass()
{

}

Show2DTexturePass::~Show2DTexturePass()
{

}

void Show2DTexturePass::Init()
{
	SetupRootSignature();
	SetupPipelineState(L"../Resources/Shaders/Tutorial12/EnvironmentShaders.hlsl", "VS_ShowTexture2D", "PS_ShowTexture2D");
}


void Show2DTexturePass::Render(FCommandContext& CommandContext,FD3D12Resource& inputTex)
{
	RenderWindow& renderWindow = RenderWindow::Get();
	FColorBuffer& BackBuffer = renderWindow.GetBackBuffer();

	// Set necessary state.
	CommandContext.SetRootSignature(m_RootSignature);
	CommandContext.SetViewportAndScissor(m_Pos.x, m_Pos.y, Size.x, Size.y);

	// Indicate that the back buffer will be used as a render target.
	CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET,true);

	const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] = { BackBuffer.GetRTV() };
	CommandContext.SetRenderTargets(1, RTVs);

	// Record commands.
	CommandContext.ClearColor(BackBuffer);
	CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandContext.SetPipelineState(m_RenderState->GetPipelineState());
	
	g_EVSConstants.ModelMatrix = FMatrix(); // identity
	g_EVSConstants.ViewProjMatrix = FMatrix::MatrixOrthoLH(1.f, 1.f, -1.f, 1.f);
	CommandContext.SetDynamicConstantBufferView(0, sizeof(EVSConstants), &g_EVSConstants);

	g_EPSConstants.Exposure = 1.0;
	CommandContext.SetDynamicConstantBufferView(1, sizeof(EPSConstants), &g_EPSConstants);

	CommandContext.Draw(3);

}


void Show2DTexturePass::SetupRootSignature()
{
	FSamplerDesc PointSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	m_RootSignature.Reset(3, 1);
	m_RootSignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_RootSignature[1].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	m_RootSignature.InitStaticSampler(0, PointSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature.Finalize(L"Show 2D Texture View RootSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void Show2DTexturePass::SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint)
{
	std::shared_ptr<FShader> Shader = FShaderMgr::Get().CreateShader(core::usc2_u8(ShaderFile), entryVSPoint, entryPSPoint, ShaderFile);
	m_RenderState = std::make_shared<RenderPipelineInfo>(Shader);
	DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;
	m_RenderState->SetupRenderTargetFormat(1, &format, DXGI_FORMAT_UNKNOWN);
	m_RenderState->SetRasterizerState(FPipelineState::RasterizerTwoSided);
	m_RenderState->SetDepthStencilState(FPipelineState::DepthStateDisabled);
	m_RenderState->SetBlendState(FPipelineState::BlendDisable);

	m_RenderState->SetupPipeline(m_RootSignature);
	m_RenderState->PipelineFinalize();
	
}

void Show2DTexturePass::SetViewportAndScissor(int32_t x, int32_t y, int32_t w, int32_t h)
{
	Size = { w,h };
	m_Pos = { x,y };
}