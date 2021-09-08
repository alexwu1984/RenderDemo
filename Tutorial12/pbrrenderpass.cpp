#include "pbrrenderpass.h"
#include "Model.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "Camera.h"
#include "EnvironmentCommon.h"
#include "CubeBuffer.h"
#include "ColorBuffer.h"
#include "Material.h"
#include "BufferManager.h"
#include "TemporalEffects.h"
#include "MotionBlur.h"

using namespace BufferManager;

PBRRenderPass::PBRRenderPass()
{

}

PBRRenderPass::~PBRRenderPass()
{

}

void PBRRenderPass::Init(const std::vector < std::shared_ptr<FRenderItem>>& ItemList, int Width, int Height,
	const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint)
{
	m_ItemList = ItemList;
	m_GameWndSize = { Width,Height };
	SetupRootSignature();
	SetupPipelineState(ShaderFile,entryVSPoint,entryPSPoint);
}


void PBRRenderPass::InitIBL(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint)
{	
	FSamplerDesc DefaultSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	m_IBLSignature.Reset(3, 1);
	m_IBLSignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_IBLSignature[1].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_IBLSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10); //scenecolor, normal, metallSpecularRoughness, AlbedoAO, velocity, irradiance, prefiltered, preintegratedGF
	m_IBLSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_IBLSignature.Finalize(L"IBL RootSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShaderDirect(ShaderFile, entryVSPoint, entryPSPoint);
	m_IBLRenderState = std::make_shared<RenderPipelineInfo>(shader);
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

void PBRRenderPass::Render(FCommandContext& CommandContext, FCamera& MainCamera, 
	FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF)
{
	CommandContext.SetRootSignature(m_MeshSignature);
	CommandContext.SetViewportAndScissor(0, 0, m_GameWndSize.x, m_GameWndSize.y);


	RenderWindow& renderWindow = RenderWindow::Get();
	//FColorBuffer& BackBuffer = renderWindow.GetBackBuffer();
	//FDepthBuffer& DepthBuffer = renderWindow.GetDepthBuffer();
	// Indicate that the back buffer will be used as a render target.
	CommandContext.TransitionResource(IrradianceCube, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(PrefilteredCube, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(PreintegratedGF, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandContext.TransitionResource(g_SceneDepthZ, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
	CommandContext.SetRenderTargets(1, &g_SceneColorBuffer.GetRTV(), g_SceneDepthZ.GetDSV());
	
	// Record commands.
	//CommandContext.ClearColor(BackBuffer);
	//CommandContext.ClearDepth(DepthBuffer);
	CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CommandContext.SetPipelineState(m_RenderState->GetPipelineState());

	for (auto Item : m_ItemList)
	{
		auto Model = Item->Model;
		if (Model)
		{
			Model->CustomDrawParam = [this,&MainCamera,&IrradianceCube,&PrefilteredCube,&PreintegratedGF, Model](FCommandContext& GfxContext, std::shared_ptr< FMaterial> Material, std::shared_ptr<FRenderItem::BasePassInfoWrapper> InfoWrapper)
			{
				g_EVSConstants.ModelMatrix = Model->GetModelMatrix();
				g_EVSConstants.ViewProjMatrix = MainCamera.GetViewProjMatrix();
				g_EVSConstants.PreviousModelMatrix = Model->GetPreviousModelMatrix();
				g_EVSConstants.PreviousViewProjMatrix = MainCamera.GetPreviousViewProjMatrix();
				g_EVSConstants.ViewportSize = Vector2f(m_GameWndSize.x, m_GameWndSize.y);
				GfxContext.SetDynamicConstantBufferView(0, sizeof(g_EVSConstants), &g_EVSConstants);

				g_PBRPSConstants.Exposure = 1;
				g_PBRPSConstants.CameraPos = MainCamera.GetPosition();
				GfxContext.SetDynamicConstantBufferView(1, sizeof(g_PBRPSConstants), &g_PBRPSConstants);

				GfxContext.SetDynamicDescriptor(2, 0, Material->GetDiffuseTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 1, Material->GetOpacityTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 2, Material->GetEmissiveTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 3, Material->GetMetallicTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 4, Material->GetRoughnessTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 5, Material->GetAmbientTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 6, Material->GetNormalTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 7, IrradianceCube.GetCubeSRV());
				GfxContext.SetDynamicDescriptor(2, 8, PrefilteredCube.GetCubeSRV());
				GfxContext.SetDynamicDescriptor(2, 9, PreintegratedGF.GetSRV());
			};

			Model->Draw(CommandContext, Item.get());

			Model->CustomDrawParam = nullptr;
		}
	}
}

void PBRRenderPass::RenderBasePass(FCommandContext& CommandContext, FCamera& MainCamera, FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF, bool Clear)
{
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
		//CommandContext.ClearColor(g_SceneColorBuffer);
		CommandContext.ClearColor(g_GBufferA);
		CommandContext.ClearColor(g_GBufferB);
		CommandContext.ClearColor(g_GBufferC);
		CommandContext.ClearColor(MotionBlur::g_VelocityBuffer);
		CommandContext.ClearDepth(g_SceneDepthZ);
	}

	for (auto Item : m_ItemList)
	{
		auto Model = Item->Model;
		if (Model)
		{
			Model->CustomDrawParam = [this, &MainCamera, &IrradianceCube, &PrefilteredCube, &PreintegratedGF, Model](FCommandContext& GfxContext, std::shared_ptr< FMaterial> Material, std::shared_ptr<FRenderItem::BasePassInfoWrapper> InfoWrapper)
			{
				g_EVSConstants.ModelMatrix = Model->GetModelMatrix();
				g_EVSConstants.ViewProjMatrix = MainCamera.GetViewProjMatrix();
				g_EVSConstants.PreviousModelMatrix = Model->GetPreviousModelMatrix();
				g_EVSConstants.PreviousViewProjMatrix = MainCamera.GetPreviousViewProjMatrix();
				g_EVSConstants.ViewportSize = Vector2f(m_GameWndSize.x, m_GameWndSize.y);


				GfxContext.SetDynamicConstantBufferView(0, sizeof(g_EVSConstants), &g_EVSConstants);

				g_PBRPSConstants.Exposure = 1;
				g_PBRPSConstants.CameraPos = MainCamera.GetPosition();
				g_PBRPSConstants.InvViewProj = MainCamera.GetViewProjMatrix().Inverse();
				g_PBRPSConstants.MaxMipLevel = PrefilteredCube.GetNumMips();
				GfxContext.SetDynamicConstantBufferView(1, sizeof(g_PBRPSConstants), &g_PBRPSConstants);

				GfxContext.SetDynamicDescriptor(2, 0, Material->GetDiffuseTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 1, Material->GetOpacityTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 2, Material->GetEmissiveTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 3, Material->GetMetallicTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 4, Material->GetRoughnessTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 5, Material->GetAmbientTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 6, Material->GetNormalTexture().GetSRV());
				GfxContext.SetDynamicDescriptor(2, 7, IrradianceCube.GetCubeSRV());
				GfxContext.SetDynamicDescriptor(2, 8, PrefilteredCube.GetCubeSRV());
				GfxContext.SetDynamicDescriptor(2, 9, PreintegratedGF.GetSRV());
			};

			Model->Draw(CommandContext, Item.get());

			Model->CustomDrawParam = nullptr;
		}
	}
}

void PBRRenderPass::RenderIBL(FCommandContext& GfxContext, FCamera& MainCamera, FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF)
{
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

	GfxContext.TransitionResource(SceneBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	GfxContext.TransitionResource(g_GBufferA, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(g_GBufferB, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(g_GBufferC, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	//GfxContext.TransitionResource(BufferManager::g_SSRBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(g_SceneDepthZ, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

	GfxContext.SetRenderTargets(1, &SceneBuffer.GetRTV());

	//g_EVSConstants.ModelMatrix = g_EVSConstants
	//g_EVSConstants.ViewProjMatrix = m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix();
	//g_EVSConstants.PreviousModelMatrix = m_Mesh->GetPreviousModelMatrix();
	//g_EVSConstants.PreviousViewProjMatrix = m_Camera.GetPreviousViewProjMatrix();
	//g_EVSConstants.ViewportSize = Vector2f(m_MainViewport.Width, m_MainViewport.Height);

	//GfxContext.SetDynamicConstantBufferView(0, sizeof(m_VSConstants), &m_VSConstants);

	g_PBRPSConstants.Exposure = 1;
	g_PBRPSConstants.CameraPos = MainCamera.GetPosition();
	g_PBRPSConstants.InvViewProj = MainCamera.GetViewProjMatrix().Inverse();
	g_PBRPSConstants.MaxMipLevel = PrefilteredCube.GetNumMips();

	GfxContext.SetDynamicConstantBufferView(1, sizeof(g_PBRPSConstants), &g_PBRPSConstants);
	GfxContext.SetDynamicDescriptor(2, 0, g_GBufferA.GetSRV());
	GfxContext.SetDynamicDescriptor(2, 1, g_GBufferB.GetSRV());
	GfxContext.SetDynamicDescriptor(2, 2, g_GBufferC.GetSRV());
	GfxContext.SetDynamicDescriptor(2, 3, g_SceneDepthZ.GetDepthSRV());
	//GfxContext.SetDynamicDescriptor(2, 4, BufferManager::g_SSRBuffer.GetSRV());

	GfxContext.SetDynamicDescriptor(2, 7, IrradianceCube.GetCubeSRV());
	GfxContext.SetDynamicDescriptor(2, 8, PrefilteredCube.GetCubeSRV());
	GfxContext.SetDynamicDescriptor(2, 9, PreintegratedGF.GetSRV());

	GfxContext.Draw(3);
}

void PBRRenderPass::Update(FCamera& MainCamera)
{
	for (auto Item : m_ItemList)
	{
		auto Model = Item->Model;
		if (Model)
		{
			Model->Update();
		}
	}
}

void PBRRenderPass::Rotate(float RotateY)
{
	for (auto Item : m_ItemList)
	{
		auto Model = Item->Model;
		if (Model)
		{
			Model->SetRotation(FMatrix::RotateY(RotateY));
		}
	}
}

void PBRRenderPass::SetupRootSignature()
{
	FSamplerDesc DefaultSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	m_MeshSignature.Reset(3, 1);
	m_MeshSignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_MeshSignature[1].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_MeshSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10); //pbr 7, irradiance, prefiltered, preintegratedGF
	m_MeshSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_MeshSignature.Finalize(L"Mesh RootSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void PBRRenderPass::SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint)
{
	std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShaderDirect(ShaderFile,entryVSPoint,entryPSPoint);
	m_RenderState = std::make_shared<RenderPipelineInfo>(shader);
	DXGI_FORMAT RTFormats[] = {
			g_SceneColorBuffer.GetFormat(), g_GBufferA.GetFormat(), g_GBufferB.GetFormat(), g_GBufferC.GetFormat(), MotionBlur::g_VelocityBuffer.GetFormat(),
	};
	m_RenderState->SetupRenderTargetFormat(5, RTFormats, g_SceneDepthZ.GetFormat());
	
	//m_RenderState->SetupRenderTargetFormat(1, &g_SceneColorBuffer.GetFormat(), g_SceneDepthZ.GetFormat());
	m_RenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerTwoSided);

	bool InitLayout = false;
	for (auto Item: m_ItemList)
	{
		auto Model = Item->Model;
		std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
		Model->GetMeshLayout(MeshLayout);
		m_RenderState->SetupPipeline(m_MeshSignature, MeshLayout);
		InitLayout = true;
		break;
	}

	Assert(InitLayout);

	m_RenderState->PipelineFinalize();
}
