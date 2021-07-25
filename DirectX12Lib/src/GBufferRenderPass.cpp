#include "gbufferrenderpass.h"
#include "Model.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "Camera.h"
#include "StringUnit.h"

GBufferRenderPass::GBufferRenderPass()
{

}

GBufferRenderPass::~GBufferRenderPass()
{

}

void GBufferRenderPass::Init(const std::vector < std::shared_ptr<FRenderItem>>& ItemList, const std::wstring& ShaderFile, int Width, int Height)
{
	m_ItemList = ItemList;
	m_GameWndSize = { Width,Height };
	SetupRootSignature();
	SetupPipelineState(ShaderFile);
}

void GBufferRenderPass::Render(FCommandContext& CommandContext)
{
	// Set necessary state.
	CommandContext.SetRootSignature(m_GBufferSignature);
	CommandContext.SetViewportAndScissor(0, 0, m_GameWndSize.x, m_GameWndSize.y);

	RenderWindow& renderWindow = RenderWindow::Get();
	// Indicate that the back buffer will be used as a render target.
	CommandContext.TransitionResource(m_AlbedoBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandContext.TransitionResource(m_NormalBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandContext.TransitionResource(m_PositionBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandContext.TransitionResource(m_DepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);

	const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] = { m_AlbedoBuffer.GetRTV(),m_NormalBuffer.GetRTV(),m_PositionBuffer.GetRTV() };
	CommandContext.SetRenderTargets(3, RTVs, m_DepthBuffer.GetDSV());

	// Record commands.
	CommandContext.ClearColor(m_AlbedoBuffer);
	CommandContext.ClearColor(m_NormalBuffer);
	CommandContext.ClearColor(m_PositionBuffer);
	CommandContext.ClearDepth(m_DepthBuffer);
	CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandContext.SetPipelineState(m_GBufferRenderState->GetPipelineState());

	for (std::shared_ptr<FRenderItem> item : m_ItemList)
	{
		item->Model->Draw(CommandContext, item.get());
	}

	CommandContext.TransitionResource(m_AlbedoBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(m_NormalBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(m_PositionBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);

	CommandContext.Flush(false);
}

void GBufferRenderPass::Update(const Vector3f& LightDir, const FMatrix& LightView, const FMatrix& LightProj, FCamera& MainCamera)
{
	for (auto Item : m_ItemList)
	{
		auto Model = Item->Model;
		if (Model)
		{
			Model->SetLightDir(LightDir);
			Model->SetLightMVP(Model->GetModelMatrix(), LightView, LightProj);

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

void GBufferRenderPass::SetupRootSignature()
{
	FSamplerDesc GBufferSamplerDesc;
	GBufferSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	GBufferSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	GBufferSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	GBufferSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	GBufferSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;

	m_GBufferSignature.Reset(3, 1);
	m_GBufferSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
	m_GBufferSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_ALL);
	m_GBufferSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_GBufferSignature.InitStaticSampler(0, GBufferSamplerDesc);
	m_GBufferSignature.Finalize(L"GBufferSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


}

void GBufferRenderPass::SetupPipelineState(const std::wstring& ShaderFile)
{
	std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShader(core::ucs2_u8(ShaderFile), ShaderFile);

	m_GBufferRenderState = std::make_shared<RenderPipelineInfo>(shader);
	const DXGI_FORMAT renderTargetFormat[] = { DXGI_FORMAT_R32G32B32A32_FLOAT,DXGI_FORMAT_R32G32B32A32_FLOAT,DXGI_FORMAT_R32G32B32A32_FLOAT };
	m_GBufferRenderState->SetupRenderTargetFormat(3, renderTargetFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
	m_GBufferRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerFront);
	m_GBufferRenderState->SetBlendState(FPipelineState::BlendDisable);

	std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
	uint32_t slot = 0;
	MeshLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	MeshLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	MeshLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	m_GBufferRenderState->SetupPipeline(m_GBufferSignature, MeshLayout);
	m_GBufferRenderState->PipelineFinalize();

	m_AlbedoBuffer.Create(L"Albedo Buffer", m_GameWndSize.x, m_GameWndSize.y, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
	m_NormalBuffer.Create(L"Normal Buffer", m_GameWndSize.x, m_GameWndSize.y, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
	m_PositionBuffer.Create(L"Position Buffer", m_GameWndSize.x, m_GameWndSize.y, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
	m_DepthBuffer.Create(L"Depth Buffer", m_GameWndSize.x, m_GameWndSize.y, DXGI_FORMAT_D24_UNORM_S8_UINT);
}
