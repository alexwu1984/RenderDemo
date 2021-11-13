#include "normalrenderpass.h"
#include "Model.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "Camera.h"

NormalRenderPass::NormalRenderPass()
{

}

NormalRenderPass::~NormalRenderPass()
{

}

void NormalRenderPass::Init(const std::vector < std::shared_ptr<FRenderItem>>& ItemList, int Width, int Height)
{
	m_ItemList = ItemList;
	m_GameWndSize = { Width,Height };
	SetupRootSignature();
	SetupPipelineState();
}

void NormalRenderPass::Render(FCommandContext& CommandContext)
{
	CommandContext.SetRootSignature(m_RootSignature);
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

	CommandContext.SetPipelineState(m_TexutreRenderState->GetPipelineState());

	for (auto Item : m_ItemList)
	{
		auto Model = Item->Model;
		if (Model)
		{
			Model->Draw(CommandContext, Item.get());
		}
	}

	CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_PRESENT);
}

void NormalRenderPass::Update(const Vector3f& LightDir, const FMatrix& View, const FMatrix& Proj, FCamera& MainCamera)
{
	for (auto Item : m_ItemList)
	{
		auto Model = Item->Model;
		if (Model)
		{
			Model->SetLightDir(LightDir);
			Model->SetLightMVP(Model->GetModelMatrix(), View, Proj);

			for (auto& PassInfo : Item->MapBasePassInfos)
			{
				PassInfo.second->BasePassInfo.modelMatrix = Model->GetModelMatrix();
				PassInfo.second->BasePassInfo.viewMatrix = MainCamera.GetViewMatrix();
				const float FovVertical = MATH_PI / 4.f;
				PassInfo.second->BasePassInfo.projectionMatrix = FMatrix::MatrixPerspectiveFovLH(FovVertical, (float)m_GameWndSize.x / m_GameWndSize.y, 0.1f, 100.f);

				memcpy(PassInfo.second->BasePassConstBuf.Map(), &PassInfo.second->BasePassInfo, sizeof(PassInfo.second->BasePassInfo));
			}
		}
	}
}

void NormalRenderPass::SetupRootSignature()
{
	FSamplerDesc DefaultSamplerDesc;
	FSamplerDesc ShadowSamplerDesc;
	ShadowSamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	ShadowSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	ShadowSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	ShadowSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	ShadowSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;

	m_RootSignature.Reset(4, 3);
	m_RootSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
	m_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_ALL);
	m_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature.InitStaticSampler(1, ShadowSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature.InitStaticSampler(2, ShadowSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature.Finalize(L"RootSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void NormalRenderPass::SetupPipelineState()
{
	std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShader("phongFragment", L"../Resources/Shaders/phongFragment.hlsl");
	m_TexutreRenderState = std::make_shared<FRenderPipelineInfo>(shader);
	m_TexutreRenderState->SetupRenderTargetFormat(1, &RenderWindow::Get().GetColorFormat(), RenderWindow::Get().GetDepthFormat());
	m_TexutreRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerFront);

	bool InitLayout = false;
	for (auto Item: m_ItemList)
	{
		auto Model = Item->Model;
		std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
		Model->GetMeshLayout(MeshLayout);
		m_TexutreRenderState->SetupPipeline(m_RootSignature, MeshLayout);
		InitLayout = true;
		break;
	}

	Assert(InitLayout);

	m_TexutreRenderState->PipelineFinalize();
}
