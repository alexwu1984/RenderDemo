#include "GltfPBRRender.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "Camera.h"
#include "CubeBuffer.h"
#include "ColorBuffer.h"
#include "Material.h"
#include "BufferManager.h"
#include "TemporalEffects.h"
#include "Shader.h"
#include "UserMarkers.h"
#include "MotionBlur.h"
#include "GLTFModel/GltfModel.h"
#include "GLTFModel/GltfMeshBuffer.h"
#include "GLTFModel/GltfMesh.h"
#include "GLTFModel/GltfMaterial.h"
#include "GLTFModel/GltfResource.h"

using namespace BufferManager;

FGlftPBRRender::FGlftPBRRender()
{

}

FGlftPBRRender::~FGlftPBRRender()
{

}

void FGlftPBRRender::InitBase(std::shared_ptr<FGLTFMode> Mode, int Width, int Height, const std::wstring& ShaderFile, 
							  const std::string& EntryVSPoint, const std::string& EntryPSPoint)
{
	m_GltfMode = Mode;
	m_GameWndSize = { Width,Height };
	SetupBaseRootSignature();
	SetupBasePipelineState(ShaderFile, EntryVSPoint, EntryPSPoint);
}

void FGlftPBRRender::InitIBL(const std::wstring& ShaderFile, const std::string& EntryVSPoint, const std::string& EntryPSPoint)
{
	FSamplerDesc DefaultSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	m_IBLSignature.Reset(3, 1);
	m_IBLSignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_IBLSignature[1].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_IBLSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10); //scenecolor, normal, metallSpecularRoughness, AlbedoAO, velocity, irradiance, prefiltered, preintegratedGF
	m_IBLSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_IBLSignature.Finalize(L"IBL RootSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShaderDirect(ShaderFile, EntryVSPoint, EntryPSPoint);
	m_IBLRenderState = std::make_shared<FRenderPipelineInfo>(shader);
	m_IBLRenderState->SetupRenderTargetFormat(1, &g_SceneColorBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);
	m_IBLRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerTwoSided);
	D3D12_BLEND_DESC BlendAdd = FPipelineState::BlendTraditional;
	BlendAdd.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	BlendAdd.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	BlendAdd.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	BlendAdd.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
	m_IBLRenderState->SetDepthStencilState(FPipelineState::DepthStateDisabled);
	m_IBLRenderState->SetBlendState(BlendAdd);
	m_IBLRenderState->SetupPipeline(m_IBLSignature);
	m_IBLRenderState->PipelineFinalize();
}

void FGlftPBRRender::RenderBasePass(FCommandContext& CommandContext, FCamera& MainCamera, FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF, bool Clear)
{
	UserMarker GpuMarker(CommandContext, "RenderBasePass");

	CommandContext.SetRootSignature(m_MeshSignature);
	CommandContext.SetPipelineState(m_RenderState->GetPipelineState());
	CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandContext.SetViewportAndScissor(0, 0, m_GameWndSize.x, m_GameWndSize.y);

	// Indicate that the back buffer will be used as a render target.
	CommandContext.TransitionResource(IrradianceCube, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(PrefilteredCube, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(PreintegratedGF, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	CommandContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandContext.TransitionResource(g_GBufferA, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandContext.TransitionResource(g_GBufferB, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandContext.TransitionResource(g_GBufferC, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandContext.TransitionResource(MotionBlur::g_VelocityBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandContext.TransitionResource(g_SceneDepthZ, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);


	D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] = {
		g_SceneColorBuffer.GetRTV(), g_GBufferA.GetRTV(), g_GBufferB.GetRTV(), g_GBufferC.GetRTV(), MotionBlur::g_VelocityBuffer.GetRTV(),
	};
	CommandContext.SetRenderTargets(5, RTVs, g_SceneDepthZ.GetDSV());

	if (Clear)
	{
		CommandContext.ClearColor(g_SceneColorBuffer);
		CommandContext.ClearColor(g_GBufferA);
		CommandContext.ClearColor(g_GBufferB);
		CommandContext.ClearColor(g_GBufferC);
		CommandContext.ClearColor(MotionBlur::g_VelocityBuffer);
		CommandContext.ClearDepth(g_SceneDepthZ);
	}

	uint32_t MeshSize = m_GltfMode->GetModelMesh().size();
	for (int32_t MeshIndex = 0; MeshIndex < MeshSize; ++MeshIndex)
	{
		std::shared_ptr<FGltfMesh> GltfMesh = m_GltfMode->GetModelMesh()[MeshIndex];
		if (!GltfMesh->IsTransparent())
		{
			std::shared_ptr<GltfMeshBuffer> MeshBuffer = GltfMesh->GetGPUBuffer();
			for (int vI = 0, slot = 0; vI < GltfMeshBuffer::VT_Max; ++vI)
			{
				CommandContext.SetVertexBuffer(slot++, MeshBuffer->VerticeBuffer[vI]->VertexBufferView());
			}
			CommandContext.SetIndexBuffer(MeshBuffer->IndexBuffer->IndexBufferView());

			FPBRMaterial* Material = GLTFResourceCast(GltfMesh->GetMaterial().get());
			FPBRPSConstants& PS = Material->GetPS();
			FGltfVSConstants& VS = MeshBuffer->GetVS();

			if (TemporalEffects::g_EnableTAA)
			{
				VS.ModelMatrix = GltfMesh->GetMeshMat();
				VS.ViewProjMatrix = MainCamera.GetViewMatrix() * TemporalEffects::HackAddTemporalAAProjectionJitter(MainCamera, m_GameWndSize.x, m_GameWndSize.y, false);
				VS.PreviousModelMatrix = GltfMesh->GetPreviousModelMatrix();
				VS.PreviousViewProjMatrix = MainCamera.GetPreviousViewMatrix() * TemporalEffects::HackAddTemporalAAProjectionJitter(MainCamera, m_GameWndSize.x, m_GameWndSize.y, true);
				VS.ViewportSize = Vector2f(m_GameWndSize.x, m_GameWndSize.y);
			}
			else
			{
				VS.ModelMatrix = GltfMesh->GetMeshMat();
				VS.ViewProjMatrix = MainCamera.GetViewProjMatrix();
				VS.PreviousModelMatrix = GltfMesh->GetPreviousModelMatrix();
				VS.PreviousViewProjMatrix = MainCamera.GetPreviousViewProjMatrix();
				VS.ViewportSize = Vector2f(m_GameWndSize.x, m_GameWndSize.y);
			}

			CommandContext.SetDynamicConstantBufferView(0, sizeof(VS), &VS);

			PS.Exposure = 1;
			PS.CameraPos = MainCamera.GetPosition();
			PS.InvViewProj = MainCamera.GetViewProjMatrix().Inverse();
			PS.MaxMipLevel = PrefilteredCube.GetNumMips();

			TemporalEffects::GetJitterOffset(PS.TemporalAAJitter, m_GameWndSize.x, m_GameWndSize.y);

			CommandContext.SetDynamicConstantBufferView(1, sizeof(PS), &PS);

			//CommandContext.SetDynamicDescriptor(2, 0, Material->GetBaseColorTexture()->GetSRV());
			//CommandContext.SetDynamicDescriptor(2, 1, Material->GetOpacityTexture().GetSRV());
			//CommandContext.SetDynamicDescriptor(2, 2, Material->GetEmissiveTexture().GetSRV());
			//CommandContext.SetDynamicDescriptor(2, 3, Material->GetMetallicTexture().GetSRV());
			//CommandContext.SetDynamicDescriptor(2, 4, Material->GetRoughnessTexture().GetSRV());
			//CommandContext.SetDynamicDescriptor(2, 5, Material->GetAmbientTexture().GetSRV());
			//CommandContext.SetDynamicDescriptor(2, 6, Material->GetNormalTexture().GetSRV());
			CommandContext.SetDynamicDescriptor(2, 7, IrradianceCube.GetCubeSRV());
			CommandContext.SetDynamicDescriptor(2, 8, PrefilteredCube.GetCubeSRV());
			CommandContext.SetDynamicDescriptor(2, 9, PreintegratedGF.GetSRV());

			CommandContext.DrawIndexed(MeshBuffer->IndexBuffer->GetElementCount());
		}
	}
}

void FGlftPBRRender::SetupBaseRootSignature()
{
	FSamplerDesc DefaultSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	m_MeshSignature.Reset(3, 1);
	m_MeshSignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_MeshSignature[1].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_MeshSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10); //pbr 7, irradiance, prefiltered, preintegratedGF
	m_MeshSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_MeshSignature.Finalize(L"Mesh RootSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void FGlftPBRRender::SetupBasePipelineState(const std::wstring& ShaderFile, const std::string& EntryVSPoint, const std::string& EntryPSPoint)
{
	std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShaderDirect(ShaderFile, EntryVSPoint, EntryPSPoint);
	m_RenderState = std::make_shared<FRenderPipelineInfo>(shader);
	DXGI_FORMAT RTFormats[] = {
			g_SceneColorBuffer.GetFormat(), g_GBufferA.GetFormat(), g_GBufferB.GetFormat(), g_GBufferC.GetFormat(), MotionBlur::g_VelocityBuffer.GetFormat(),
	};
	m_RenderState->SetupRenderTargetFormat(5, RTFormats, g_SceneDepthZ.GetFormat());

	m_RenderState->SetRasterizerState(FPipelineState::RasterizerTwoSided);
	m_RenderState->SetBlendState(FPipelineState::BlendDisable);
	m_RenderState->SetDepthStencilState(FPipelineState::DepthStateReadWrite);

	bool InitLayout = false;

	const auto& MeshList = m_GltfMode->GetModelMesh();
	for (auto Item: MeshList)
	{
		if (Item->GetGPUBuffer())
		{
			std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
			Item->GetGPUBuffer()->GetMeshLayout(MeshLayout);
			m_RenderState->SetupPipeline(m_MeshSignature, MeshLayout);
			InitLayout = true;
			break;
		}
	}

	Assert(InitLayout);

	m_RenderState->PipelineFinalize();
}

void FGlftPBRRender::RenderIBL(FCommandContext& GfxContext, FCamera& MainCamera, FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF)
{
	UserMarker GpuMarker(GfxContext, "RenderIBL");
	// Set necessary state.
	GfxContext.SetRootSignature(m_IBLSignature);
	GfxContext.SetPipelineState(m_IBLRenderState->GetPipelineState());
	GfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// jitter offset
	GfxContext.SetViewportAndScissor(0, 0, m_GameWndSize.x, m_GameWndSize.y);

	FColorBuffer& SceneBuffer = g_SceneColorBuffer;

	// Indicate that the back buffer will be used as a render target.
	GfxContext.TransitionResource(IrradianceCube, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(PrefilteredCube, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(PreintegratedGF, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	GfxContext.TransitionResource(BufferManager::g_SSRBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(SceneBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	GfxContext.TransitionResource(g_GBufferA, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(g_GBufferB, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(g_GBufferC, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(g_SceneDepthZ, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

	GfxContext.SetRenderTargets(1, &SceneBuffer.GetRTV());

	m_IBLPS.Exposure = 1;
	m_IBLPS.CameraPos = MainCamera.GetPosition();
	m_IBLPS.InvViewProj = MainCamera.GetViewProjMatrix().Inverse();
	m_IBLPS.MaxMipLevel = PrefilteredCube.GetNumMips();

	GfxContext.SetDynamicConstantBufferView(1, sizeof(m_IBLPS), &m_IBLPS);
	GfxContext.SetDynamicDescriptor(2, 0, g_GBufferA.GetSRV());
	GfxContext.SetDynamicDescriptor(2, 1, g_GBufferB.GetSRV());
	GfxContext.SetDynamicDescriptor(2, 2, g_GBufferC.GetSRV());
	GfxContext.SetDynamicDescriptor(2, 3, g_SceneDepthZ.GetDepthSRV());
	GfxContext.SetDynamicDescriptor(2, 4, BufferManager::g_SSRBuffer.GetSRV());

	GfxContext.SetDynamicDescriptor(2, 7, IrradianceCube.GetCubeSRV());
	GfxContext.SetDynamicDescriptor(2, 8, PrefilteredCube.GetCubeSRV());
	GfxContext.SetDynamicDescriptor(2, 9, PreintegratedGF.GetSRV());

	GfxContext.Draw(3);
}

