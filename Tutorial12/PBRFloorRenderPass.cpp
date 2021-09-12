#include "PBRFloorRenderPass.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "Shader.h"
#include "RenderPipelineInfo.h"
#include "BufferManager.h"
#include "MotionBlur.h"
#include "EnvironmentCommon.h"

using namespace BufferManager;

PBRFloorRenderPass::PBRFloorRenderPass()
{

}

PBRFloorRenderPass::~PBRFloorRenderPass()
{

}

void PBRFloorRenderPass::Init()
{

	m_FloorAlpha.LoadFromFile(L"../Resources/Models/harley/textures/Floor_Alpha.jpg", false);
	m_FloorAlbedo.LoadFromFile(L"../Resources/Models/harley/textures/default.png", true);

	SetupRootSignature();
	SetupPipelineState();
}

void PBRFloorRenderPass::RenderBasePass(FCommandContext& CommandContext)
{
	CommandContext.SetPipelineState(m_RenderState->GetPipelineState());

	// draw floor
	g_EVSConstants.ModelMatrix = FMatrix::ScaleMatrix(5.f);
	CommandContext.SetDynamicConstantBufferView(0, sizeof(g_EVSConstants), &g_EVSConstants);

	CommandContext.SetDynamicDescriptor(2, 0, m_FloorAlbedo.GetSRV());
	CommandContext.SetDynamicDescriptor(2, 1, m_FloorAlpha.GetSRV());
	CommandContext.Draw(6);
}

void PBRFloorRenderPass::SetupRootSignature()
{
	FSamplerDesc DefaultSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	m_MeshSignature.Reset(3, 1);
	m_MeshSignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_MeshSignature[1].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_MeshSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10); //pbr 7, irradiance, prefiltered, preintegratedGF
	m_MeshSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_MeshSignature.Finalize(L"Mesh RootSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void PBRFloorRenderPass::SetupPipelineState()
{
	std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShaderDirect(L"../Resources/Shaders/PBR.hlsl", "VS_PBR_Floor", "PS_PBR_Floor");
	m_RenderState = std::make_shared<RenderPipelineInfo>(shader);
	m_RenderState->SetRasterizerState(FPipelineState::RasterizerTwoSided);
	m_RenderState->SetBlendState(FPipelineState::BlendTraditional);
	m_RenderState->SetDepthStencilState(FPipelineState::DepthStateReadWrite);

	DXGI_FORMAT RTFormats[] = {
	g_SceneColorBuffer.GetFormat(), g_GBufferA.GetFormat(), g_GBufferB.GetFormat(), g_GBufferC.GetFormat(), MotionBlur::g_VelocityBuffer.GetFormat(),
	};
	m_RenderState->SetupRenderTargetFormat(5, RTFormats, g_SceneDepthZ.GetFormat());
	m_RenderState->SetupPipeline(m_MeshSignature);
	m_RenderState->PipelineFinalize();
}
