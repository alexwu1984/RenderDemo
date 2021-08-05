#include "pbrrenderpass.h"
#include "Model.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "Camera.h"
#include "EnvironmentCommon.h"

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

void PBRRenderPass::Render(FCommandContext& CommandContext)
{
	CommandContext.SetRootSignature(m_MeshSignature);
	CommandContext.SetViewportAndScissor(0, 0, m_GameWndSize.x, m_GameWndSize.y);


	RenderWindow& renderWindow = RenderWindow::Get();
	FColorBuffer& BackBuffer = renderWindow.GetBackBuffer();
	FDepthBuffer& DepthBuffer = renderWindow.GetDepthBuffer();
	// Indicate that the back buffer will be used as a render target.
	CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandContext.TransitionResource(DepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
	CommandContext.SetRenderTargets(1, &BackBuffer.GetRTV(), DepthBuffer.GetDSV());

	// Record commands.
	CommandContext.ClearColor(BackBuffer);
	CommandContext.ClearDepth(DepthBuffer);
	CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CommandContext.SetPipelineState(m_RenderState->GetPipelineState());

	for (auto Item : m_ItemList)
	{
		auto Model = Item->Model;
		if (Model)
		{
			Model->CustomDrawParam = [this](FCommandContext& GfxContext, std::shared_ptr< FMaterial> material, std::shared_ptr<FRenderItem::BasePassInfoWrapper> InfoWrapper)
			{
				g_EVSConstants.ModelMatrix = InfoWrapper->BasePassInfo.modelMatrix;
				g_EVSConstants.ViewProjMatrix = InfoWrapper->BasePassInfo.viewMatrix * InfoWrapper->BasePassInfo.projectionMatrix;
				GfxContext.SetDynamicConstantBufferView(0, sizeof(g_EVSConstants), &g_EVSConstants);

				//g_EPSConstants.Exposure = 1;
				//g_EPSConstants.CameraPos = m_Camera.GetPosition();
				//GfxContext.SetDynamicConstantBufferView(1, sizeof(g_EPSConstants), &g_EPSConstants);

				//GfxContext.SetDynamicDescriptor(2, 7, m_IrradianceCube.GetCubeSRV());
				//GfxContext.SetDynamicDescriptor(2, 8, m_PrefilteredCube.GetCubeSRV());
				//GfxContext.SetDynamicDescriptor(2, 9, m_PreintegratedGF.GetSRV());
			};

			Model->Draw(CommandContext, Item.get());

			Model->CustomDrawParam = nullptr;
		}
	}

	CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_PRESENT);
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
	m_RenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerFront);

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
