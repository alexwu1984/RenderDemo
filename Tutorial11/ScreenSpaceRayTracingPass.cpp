#include "ScreenSpaceRayTracingPass.h"
#include "Model.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "Camera.h"
#include "StringUnit.h"

ScreenSpaceRayTracingPass::ScreenSpaceRayTracingPass()
{

}

ScreenSpaceRayTracingPass::~ScreenSpaceRayTracingPass()
{

}

void ScreenSpaceRayTracingPass::Init(const std::vector < std::shared_ptr<FRenderItem>>& ItemList, const std::wstring& ShaderFile, int Width, int Height)
{
	m_ItemList = ItemList;
	m_GameWndSize = { Width,Height };
	SetupRootSignature();
	SetupPipelineState(ShaderFile);

	
}

void ScreenSpaceRayTracingPass::Render(FCommandContext& CommandContext, FDepthBuffer& Depth, FColorBuffer Albedo)
{
	// Set necessary state.
	CommandContext.SetRootSignature(m_Signature);
	CommandContext.SetViewportAndScissor(0, 0, m_GameWndSize.x, m_GameWndSize.y);

	RenderWindow& renderWindow = RenderWindow::Get();
	// Indicate that the back buffer will be used as a render target.
	CommandContext.TransitionResource(m_AlbedoBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CommandContext.TransitionResource(m_DepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);

	const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] = { m_AlbedoBuffer.GetRTV() };
	CommandContext.SetRenderTargets(1, RTVs, m_DepthBuffer.GetDSV());

	// Record commands.
	CommandContext.ClearColor(m_AlbedoBuffer);
	CommandContext.ClearDepth(m_DepthBuffer);
	CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandContext.SetPipelineState(m_RenderState->GetPipelineState());

	for (std::shared_ptr<FRenderItem> item : m_ItemList)
	{
		item->Model->CustomDrawParam = [&Depth, Albedo, this](FCommandContext& Context, std::shared_ptr< FMaterial> Material, std::shared_ptr<FRenderItem::BasePassInfoWrapper> BasePass)
		{
			Context.SetDynamicDescriptor(0, 0, BasePass->BasePassCpuHandle);
			Context.SetDynamicDescriptor(1, 0, m_SSRInfoCpuHandle);
			Context.SetDynamicDescriptor(2, 0, Depth.GetDepthSRV());
			Context.SetDynamicDescriptor(3, 0, Albedo.GetSRV());
		};
		item->Model->Draw(CommandContext, item.get());
		item->Model->CustomDrawParam = nullptr;
	}

	CommandContext.TransitionResource(m_AlbedoBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,true);

	CommandContext.Flush(false);
}

void ScreenSpaceRayTracingPass::Update(FCamera& MainCamera)
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
				const float FovVertical = MATH_PI / 4.f;
				PassInfo.second->BasePassInfo.projectionMatrix = FMatrix::MatrixPerspectiveFovLH(FovVertical, (float)m_GameWndSize.x / m_GameWndSize.y, 0.1f, 100.f);

				memcpy(PassInfo.second->BasePassConstBuf.Map(), &PassInfo.second->BasePassInfo, sizeof(PassInfo.second->BasePassInfo));
			}
		}
	}

	m_SSRInfo.CameraPosInWorldSpace = MainCamera.GetPosition();
	memcpy(m_SSRInfoConstBuf.Map(), &m_SSRInfo, sizeof(m_SSRInfo));
}

void ScreenSpaceRayTracingPass::SetupRootSignature()
{
	FSamplerDesc SamplerDesc;
	SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;

	m_Signature.Reset(4, 1);
	m_Signature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
	m_Signature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_ALL);
	m_Signature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
	m_Signature[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_Signature.InitStaticSampler(0, SamplerDesc);
	m_Signature.Finalize(L"ScreenSpaceRayTracing", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


}

void ScreenSpaceRayTracingPass::SetupPipelineState(const std::wstring& ShaderFile)
{
	std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShader(core::ucs2_u8(ShaderFile), ShaderFile);

	m_RenderState = std::make_shared<RenderPipelineInfo>(shader);
	const DXGI_FORMAT renderTargetFormat[] = { DXGI_FORMAT_R32G32B32A32_FLOAT };
	m_RenderState->SetupRenderTargetFormat(1, renderTargetFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
	m_RenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerFront);
	m_RenderState->SetBlendState(FPipelineState::BlendDisable);

	std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
	uint32_t slot = 0;
	MeshLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	MeshLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	MeshLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	m_RenderState->SetupPipeline(m_Signature, MeshLayout);
	m_RenderState->PipelineFinalize();

	m_AlbedoBuffer.Create(L"Albedo Buffer", m_GameWndSize.x, m_GameWndSize.y, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
	m_DepthBuffer.Create(L"Depth Buffer", m_GameWndSize.x, m_GameWndSize.y, DXGI_FORMAT_D24_UNORM_S8_UINT);

	m_SSRInfoConstBuf.CreateUpload(L"SSRInfo", sizeof(SSRInfo));
	m_SSRInfoCpuHandle = m_SSRInfoConstBuf.CreateConstantBufferView(0, sizeof(SSRInfo));
	m_SSRInfo.WindowWidth = m_GameWndSize.x;
	m_SSRInfo.WindowHeight = m_GameWndSize.y;
	memcpy(m_SSRInfoConstBuf.Map(), &m_SSRInfo, sizeof(m_SSRInfo));
}
