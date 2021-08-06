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

void PBRRenderPass::Render(FCommandContext& CommandContext, FCamera& MainCamera, 
	FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF)
{
	CommandContext.SetRootSignature(m_MeshSignature);
	CommandContext.SetViewportAndScissor(0, 0, m_GameWndSize.x, m_GameWndSize.y);


	RenderWindow& renderWindow = RenderWindow::Get();
	FColorBuffer& BackBuffer = renderWindow.GetBackBuffer();
	FDepthBuffer& DepthBuffer = renderWindow.GetDepthBuffer();
	// Indicate that the back buffer will be used as a render target.
	CommandContext.TransitionResource(IrradianceCube, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(PrefilteredCube, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(PreintegratedGF, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandContext.TransitionResource(DepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
	CommandContext.SetRenderTargets(1, &BackBuffer.GetRTV(), DepthBuffer.GetDSV());
	
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
			Model->CustomDrawParam = [this,&MainCamera,&IrradianceCube,&PrefilteredCube,&PreintegratedGF](FCommandContext& GfxContext, std::shared_ptr< FMaterial> Material, std::shared_ptr<FRenderItem::BasePassInfoWrapper> InfoWrapper)
			{
				g_EVSConstants.ModelMatrix = InfoWrapper->BasePassInfo.modelMatrix;
				g_EVSConstants.ViewProjMatrix = InfoWrapper->BasePassInfo.viewMatrix * InfoWrapper->BasePassInfo.projectionMatrix;
				GfxContext.SetDynamicConstantBufferView(0, sizeof(g_EVSConstants), &g_EVSConstants);

				g_EPSConstants.Exposure = 1;
				g_EPSConstants.CameraPos = MainCamera.GetPosition();
				GfxContext.SetDynamicConstantBufferView(1, sizeof(g_EPSConstants), &g_EPSConstants);

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

void PBRRenderPass::Update(FCamera& MainCamera)
{
	for (auto Item : m_ItemList)
	{
		auto Model = Item->Model;
		if (Model)
		{
			for (auto& PassInfo : Item->MapBasePassInfos)
			{
				PassInfo.second->BasePassInfo.modelMatrix = Model->GetModelMatrix();
				PassInfo.second->BasePassInfo.viewMatrix = MainCamera.GetViewMatrix();
				PassInfo.second->BasePassInfo.projectionMatrix = MainCamera.GetProjectionMatrix();
			}
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
	m_RenderState->SetupRenderTargetFormat(1, &RenderWindow::Get().GetColorFormat(), RenderWindow::Get().GetDepthFormat());
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
