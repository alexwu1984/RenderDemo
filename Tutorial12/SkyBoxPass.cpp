#include "SkyBoxPass.h"
#include "SkyBox.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "StringUnit.h"
#include "Shader.h"
#include "CubeBuffer.h"
#include "BufferManager.h"

using namespace BufferManager;

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
	m_ShaderFile = ShaderFile;
	m_EntryVSPoint = entryVSPoint;
	m_EntryPSPoint = entryPSPoint;
}

void SkyBoxPass::Render(FCommandContext& GfxContext, FCamera& MainCamera,FCubeBuffer& CubeBuffer, bool clear)
{
	if (!m_RenderState)
	{
		SetupPipelineState(FPipelineState::DepthStateReadOnly);
	}
	GfxContext.SetRootSignature(m_SkySignature);
	GfxContext.SetPipelineState(m_RenderState->GetPipelineState());
	GfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GfxContext.SetViewportAndScissor(0, 0, m_Size.x, m_Size.y);

	// Indicate that the back buffer will be used as a render target.
	GfxContext.TransitionResource(CubeBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	GfxContext.TransitionResource(g_SceneDepthZ, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);

	GfxContext.SetRenderTargets(1, &g_SceneColorBuffer.GetRTV(), g_SceneDepthZ.GetDSV());

	if (clear)
	{
		GfxContext.ClearColor(g_SceneColorBuffer);
		GfxContext.ClearDepth(g_SceneDepthZ);
	}

	g_EVSConstants.ModelMatrix = FMatrix::TranslateMatrix(MainCamera.GetPosition()); // move with camera
	g_EVSConstants.ViewProjMatrix = MainCamera.GetViewMatrix() * MainCamera.GetProjectionMatrix();
	GfxContext.SetDynamicConstantBufferView(0, sizeof(g_EVSConstants), &g_EVSConstants);
	GfxContext.SetDynamicConstantBufferView(1, sizeof(PBR_Constants), &PBR_Constants);

	GfxContext.SetDynamicDescriptor(2, 0, CubeBuffer.GetCubeSRV());

	m_SkyBox->Draw(GfxContext);
}

void SkyBoxPass::ShowCubeMapDebugView(FCommandContext& GfxContext, FCubeBuffer& CubeBuffer, float Exposure, int MipLevel, const std::vector<Vector3f>& SHCoeffs, int SHDegree)
{
	if (!m_RenderState)
	{
		SetupPipelineState(FPipelineState::DepthStateDisabled);
	}
	// Set necessary state.
	GfxContext.SetRootSignature(m_SkySignature);
	GfxContext.SetPipelineState(m_RenderState->GetPipelineState());
	GfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	uint32_t Size = std::min(m_Size.x, m_Size.y);
	//Size = std::min(Size, CubeBuffer.GetWidth());
	GfxContext.SetViewportAndScissor((m_Size.x - Size) / 2, (m_Size.y - Size) / 2, Size, Size);


	// Indicate that the back buffer will be used as a render target.
	GfxContext.TransitionResource(CubeBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

	GfxContext.SetRenderTargets(1, &g_SceneColorBuffer.GetRTV());

	GfxContext.ClearColor(g_SceneColorBuffer);

	g_EVSConstants.ModelMatrix = FMatrix(); // identity
	g_EVSConstants.ViewProjMatrix = FMatrix::MatrixOrthoLH(1.f, 1.f, -1.f, 1.f);
	GfxContext.SetDynamicConstantBufferView(0, sizeof(g_EVSConstants), &g_EVSConstants);

	PBR_Constants.MipLevel = MipLevel;
	PBR_Constants.Exposure = Exposure;
	PBR_Constants.Degree = SHDegree;
	for (int i = 0; i < SHCoeffs.size(); ++i)
	{
		PBR_Constants.Coeffs[i] = SHCoeffs[i];
	}
	
	GfxContext.SetDynamicConstantBufferView(1, sizeof(PBR_Constants), &PBR_Constants);

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

void SkyBoxPass::SetupPipelineState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc)
{
	std::shared_ptr<FShader> Shader = FShaderMgr::Get().CreateShaderDirect(m_ShaderFile, m_EntryVSPoint, m_EntryPSPoint);
	m_RenderState = std::make_shared<RenderPipelineInfo>(Shader);

	m_RenderState->SetupRenderTargetFormat(1, &g_SceneColorBuffer.GetFormat(), g_SceneDepthZ.GetFormat());
	m_RenderState->SetRasterizerState(FPipelineState::RasterizerTwoSided);
	m_RenderState->SetDepthStencilState(DepthStencilDesc);
	m_RenderState->SetBlendState(FPipelineState::BlendDisable);

	std::vector<D3D12_INPUT_ELEMENT_DESC> SkyBoxLayout;
	m_SkyBox->GetMeshLayout(SkyBoxLayout);

	m_RenderState->SetupPipeline(m_SkySignature, SkyBoxLayout);
	m_RenderState->PipelineFinalize();
}
