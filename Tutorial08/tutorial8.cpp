#include "ApplicationWin32.h"
#include "Game.h"
#include "Common.h"
#include "MathLib.h"
#include "Camera.h"
#include "CommandQueue.h"
#include "D3D12RHI.h"
#include "d3dx12.h"
#include "RenderWindow.h"
#include "CommandListManager.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "GpuBuffer.h"
#include "PipelineState.h"
#include "DirectXTex.h"
#include "Texture.h"
#include "SamplerManager.h"
#include "Model.h"
#include "Shader.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "Geometry.h"

#include <dxgi1_4.h>
#include <chrono>
#include <iostream>

constexpr int32_t RSM_BUFFER_SIZE = 256;
extern FCommandListManager g_CommandListManager;

class Tutorial8 : public FGame
{
public:
	Tutorial8(const GameDesc& Desc) : FGame(Desc)
		, m_Camera(Vector3f(0.f, 0.f, -1.f), Vector3f(0.f, 0.0f, 0.f), Vector3f(0.f, 1.f, 0.f))
	{
	}

	virtual void OnStartup()
	{
		SetupRootSignature();
		SetupRenderItem();
		SetupGBuffer();
		SetupDebugItem();
		SetupRSMBuffer();
	}

	void SetupRenderItem()
	{
		m_lightPos = Vector3f(5, 5, 5);
		float radius = sqrtf(5 * 5 + 10 * 10);
		m_LightDir = Vector3f(0, 1, 0) - m_lightPos;

		std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShader("phongColorFragment", L"../Resources/Shaders/phongColorFragment.hlsl");
		m_ColorRenderState = std::make_shared<RenderPipelineInfo>(shader);
		m_ColorRenderState->SetupRenderTargetFormat(1, &RenderWindow::Get().GetColorFormat(), RenderWindow::Get().GetDepthFormat());
		m_ColorRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerFront);


		std::shared_ptr<FRenderItem> FloorItem = std::make_shared<FRenderItem>();
		FloorItem->Init(L"../Resources/Models/floor/floor.obj");
		FloorItem->Model->SetPosition(Vector3f(0, -2, 0));
		FloorItem->Model->SetLightDir(m_LightDir);
		FloorItem->Model->SetLightIntensity(0.5f);
		FloorItem->Model->SetColor(Vector3f(0.8f, 0.8f, 0.8f));
		FloorItem->Model->SetLightColor(Vector4f(0, 0, 1, 1));
		FloorItem->PiplelineInfo = m_ColorRenderState;
		//m_AllItems.push_back(FloorItem);

		std::shared_ptr<FRenderItem> WallItem1 = std::make_shared<FRenderItem>();
		WallItem1->Init(L"../Resources/Models/floor/quad.obj");
		WallItem1->Model->SetPosition(Vector3f(0, 2, 4));
		WallItem1->Model->SetLightDir(m_LightDir);
		WallItem1->Model->SetLightIntensity(0.5);
		WallItem1->Model->SetRotation(FMatrix::RotateX(ConvertToRadians(270)));
		WallItem1->Model->SetColor(Vector3f(0, 1, 0));
		WallItem1->Model->SetLightColor(Vector4f(0, 0, 1, 1));
		WallItem1->PiplelineInfo = m_ColorRenderState;
		//m_AllItems.push_back(WallItem1);

		std::shared_ptr<FRenderItem> WallItem2 = std::make_shared<FRenderItem>();
		WallItem2->Init(L"../Resources/Models/floor/quad.obj");
		WallItem2->Model->SetPosition(Vector3f(4, 2, 0));
		WallItem2->Model->SetLightDir(m_LightDir);
		WallItem2->Model->SetLightIntensity(0.5);
		WallItem2->Model->SetRotation(FMatrix::RotateZ(ConvertToRadians(90)));
		WallItem2->Model->SetColor(Vector3f(1, 0, 0));
		WallItem2->PiplelineInfo = m_ColorRenderState;
		WallItem2->Model->SetLightColor(Vector4f(0, 0, 1, 1));
		//m_AllItems.push_back(WallItem2);


		std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
		FloorItem->Model->GetMeshLayout(MeshLayout);

		m_ColorRenderState->SetupPipeline(m_RootSignature, MeshLayout);
		m_ColorRenderState->PipelineFinalize();

		std::shared_ptr<FRenderItem> ActorItem = std::make_shared<FRenderItem>();
		ActorItem->Init(L"../Resources/Models/HalfCornellBoxAndBuddha/HalfCornellBoxAndBuddha.obj");
		ActorItem->Model->SetPosition(Vector3f(3, -2, 3));
		ActorItem->Model->SetLightDir(m_LightDir);
		ActorItem->Model->SetRotation(FMatrix::RotateY(ConvertToRadians(45)));
		ActorItem->Model->SetLightIntensity(0.5);
		ActorItem->Model->SetScale(1.5);
		m_AllItems.push_back(ActorItem);
		m_MainRadius = ActorItem->Model->GetBoundBox().Extents.Length()*3;
		

		shader = FShaderMgr::Get().CreateShader("phongFragment", L"../Resources/Shaders/phongFragment.hlsl");
		m_TexutreRenderState = std::make_shared<RenderPipelineInfo>(shader);
		m_TexutreRenderState->SetupRenderTargetFormat(1, &RenderWindow::Get().GetColorFormat(), RenderWindow::Get().GetDepthFormat());
		m_TexutreRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerFront);
		ActorItem->PiplelineInfo = m_TexutreRenderState;

		ActorItem->Model->GetMeshLayout(MeshLayout);
		m_TexutreRenderState->SetupPipeline(m_RootSignature, MeshLayout);
		m_TexutreRenderState->PipelineFinalize();
	}

	void SetupRootSignature()
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
		m_RootSignature.InitStaticSampler(0, ShadowSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature.InitStaticSampler(1, ShadowSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature.InitStaticSampler(2, ShadowSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature.Finalize(L"RootSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		m_GeometrySignature.Reset(2, 1);
		m_GeometrySignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX);
		m_GeometrySignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
		m_GeometrySignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		m_GeometrySignature.Finalize(L"GeometrySignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	}

	void SetupGBuffer()
	{
		FSamplerDesc GBufferSamplerDesc;
		GBufferSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
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

		std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShader("GBuffer", L"../Resources/Shaders/GBuffer.hlsl");
		
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


		m_AlbedoBuffer.Create(L"Albedo Buffer", m_GameDesc.Width, m_GameDesc.Height, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_NormalBuffer.Create(L"Normal Buffer", m_GameDesc.Width, m_GameDesc.Height, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_PositionBuffer.Create(L"Position Buffer", m_GameDesc.Width, m_GameDesc.Height, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_DepthBuffer.Create(L"Depth Buffer", m_GameDesc.Width, m_GameDesc.Height, DXGI_FORMAT_D24_UNORM_S8_UINT);
	}

	void SetupDebugItem()
	{
		std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShader("ScreenDebug", L"../Resources/Shaders/ScreenDebug.hlsl");
		m_DebugRenderState = std::make_shared<RenderPipelineInfo>(shader);
		m_DebugRenderState->SetupRenderTargetFormat(1, &RenderWindow::Get().GetColorFormat(), RenderWindow::Get().GetDepthFormat());
		m_DebugRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerDefault);

		m_DebugItem.Init();
		m_DebugItem.PiplelineInfo = m_DebugRenderState;
		m_DebugItem.Geo = std::make_shared<class FGeometry>();
		m_DebugItem.Geo->CreateQuad(-1, 1, 2, 2, 0);

		std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
		m_DebugItem.Geo->GetMeshLayout(MeshLayout);

		m_DebugRenderState->SetupPipeline(m_GeometrySignature, MeshLayout);
		m_DebugRenderState->PipelineFinalize();
	}

	void SetupRSMBuffer()
	{
		std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShader("RSMBuffer", L"../Resources/Shaders/RSMBuffer.hlsl");
		m_RSMBufferRenderState = std::make_shared<RenderPipelineInfo>(shader);
		const DXGI_FORMAT renderTargetFormat[] = { DXGI_FORMAT_R32G32B32A32_FLOAT,DXGI_FORMAT_R32G32B32A32_FLOAT,DXGI_FORMAT_R32G32B32A32_FLOAT };
		m_RSMBufferRenderState->SetupRenderTargetFormat(3, renderTargetFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		m_RSMBufferRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerFront);
		m_RSMBufferRenderState->SetBlendState(FPipelineState::BlendDisable);

		std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
		uint32_t slot = 0;
		MeshLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		MeshLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		MeshLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

		m_RSMBufferRenderState->SetupPipeline(m_GBufferSignature, MeshLayout);
		m_RSMBufferRenderState->PipelineFinalize();

		m_RSMFluxBuffer.Create(L"RSMFluxBuffer", RSM_BUFFER_SIZE, RSM_BUFFER_SIZE, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_RSMNormalBuffer.Create(L"RSMNormalBuffer", RSM_BUFFER_SIZE, RSM_BUFFER_SIZE, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_RSMPositionBuffer.Create(L"RSMPositionBuffer", RSM_BUFFER_SIZE, RSM_BUFFER_SIZE, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_RSMDepthBuffer.Create(L"RSMDepthBuffer", RSM_BUFFER_SIZE, RSM_BUFFER_SIZE, DXGI_FORMAT_D24_UNORM_S8_UINT);
		m_RSMOutputBuffer.Create(L"RSMOutputBuffer", m_GameDesc.Width, m_GameDesc.Height, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);

		FSamplerDesc GBufferSamplerDesc;
		GBufferSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		GBufferSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		GBufferSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		GBufferSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		GBufferSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;

		m_RSMCSSignature.Reset(8, 1);
		m_RSMCSSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1);
		m_RSMCSSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
		m_RSMCSSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		m_RSMCSSignature[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);
		m_RSMCSSignature[4].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1);
		m_RSMCSSignature[5].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 1);
		m_RSMCSSignature[6].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 1);
		m_RSMCSSignature[7].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
		m_RSMCSSignature.InitStaticSampler(0, GBufferSamplerDesc);
		m_RSMCSSignature.Finalize(L"RSMCSSignature");

		m_RSMShader = FShaderMgr::Get().CreateShader("ShadingWithRSM", "cs_main", L"../Resources/Shaders/ShadingWithRSM_CS.hlsl");
		m_RSMCSPSO.SetRootSignature(m_RSMCSSignature);
		m_RSMCSPSO.SetComputeShader(CD3DX12_SHADER_BYTECODE(m_RSMShader->GetComputeShader().Get()));
		m_RSMCSPSO.Finalize();

		std::default_random_engine e;
		std::uniform_real_distribution<float> u(0, 1);
		for (int i = 0; i < m_VPLNum; ++i)
		{
			float xi1 = u(e);
			float xi2 = u(e);
			m_RSMBasePassInfo.VPLsSampleCoordsAndWeights[i] = { xi1 * std::sin(2 * MATH_PI * xi2), xi1 * std::cos(2 * MATH_PI * xi2), xi1 * xi1, 0 };
		}

		m_RSMBasePassConstBuf.CreateUpload(L"RSMBasePassInfo", sizeof(m_RSMBasePassInfo));
		m_RSMBasePassCpuHandle = m_RSMBasePassConstBuf.CreateConstantBufferView(0, sizeof(m_RSMBasePassInfo));
	}

	virtual void OnUpdate()
	{
		Vector3f lightPos = SphericalToCartesian(1.0f, m_SunTheta, m_SunPhi);
		Vector3f lightUp(0.0f, 1.0f, 0.0f);
		Vector3f targetPos(0, 1, 0);
		FCamera lightCamera(lightPos, targetPos, lightUp);

		FMatrix lightView = lightCamera.GetViewMatrix();
		Vector3f sphereCenterLS = lightView.TransformPosition(targetPos);

		// Ortho frustum in light space encloses scene.
		float l = sphereCenterLS.x - m_MainRadius;
		float b = sphereCenterLS.y - m_MainRadius;
		float n = sphereCenterLS.z - m_MainRadius;
		float r = sphereCenterLS.x + m_MainRadius;
		float t = sphereCenterLS.y + m_MainRadius;
		float f = sphereCenterLS.z + m_MainRadius;

		FMatrix lightOrt = FMatrix::MatrixOrthographicOffCenterLH(l, r, b, t, n, f);

		//m_LightDir = targetPos - lightPos;
		m_LightVP = lightView * lightOrt;

		for (std::shared_ptr<FRenderItem> item : m_AllItems)
		{	
			item->Model->SetLightDir(m_LightDir.Normalize());
			item->Model->SetLightMVP(item->Model->GetModelMatrix(), lightView, lightOrt);

			for (auto& PassInfo : item->MapBasePassInfos)
			{
				PassInfo.second->BasePassInfo.modelMatrix = item->Model->GetModelMatrix();
				PassInfo.second->BasePassInfo.viewMatrix = m_Camera.GetViewMatrix();
				const float FovVertical = MATH_PI / 4.f;
				PassInfo.second->BasePassInfo.projectionMatrix = FMatrix::MatrixPerspectiveFovLH(FovVertical, (float)GetDesc().Width / GetDesc().Height, 0.1f, 100.f);

				memcpy(PassInfo.second->BasePassConstBuf.Map(), &PassInfo.second->BasePassInfo, sizeof(PassInfo.second->BasePassInfo));
			}

		}
	}

	virtual void OnRender()
	{
		tEnd = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::milli>(tEnd - tStart).count();
		if (time < (1000.0f / 400.0f))
		{
			return;
		}

		FCommandContext& CommandContext = FCommandContext::Begin();
		tStart = std::chrono::high_resolution_clock::now();

		// Update Uniforms
		m_DeltaTime = 0.001f * time;
		m_elapsedTime += m_DeltaTime;
		m_elapsedTime = fmodf(m_elapsedTime, 6.283185307179586f);

		RenderToGBuffer(CommandContext);
		RenderToRSMBuffer(CommandContext);
		ShadingWithRSM(CommandContext);
		RenderDebug(CommandContext);
		

		CommandContext.Finish(true);

		RenderWindow::Get().Present();
	}

	virtual void OnShutdown()
	{

	}


	void RenderToGBuffer(FCommandContext& CommandContext)
	{
		// Set necessary state.
		CommandContext.SetRootSignature(m_GBufferSignature);
		CommandContext.SetViewportAndScissor(0, 0, m_GameDesc.Width, m_GameDesc.Height);

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

		for (std::shared_ptr<FRenderItem> item : m_AllItems)
		{	
			item->Model->Draw(CommandContext,item.get());
		}

		CommandContext.TransitionResource(m_AlbedoBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		CommandContext.TransitionResource(m_NormalBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		CommandContext.TransitionResource(m_PositionBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,true);

		CommandContext.Flush(false);
	}

	void RenderToRSMBuffer(FCommandContext& CommandContext)
	{
		CommandContext.SetRootSignature(m_GBufferSignature);
		CommandContext.SetViewportAndScissor(0, 0, RSM_BUFFER_SIZE, RSM_BUFFER_SIZE);

		RenderWindow& renderWindow = RenderWindow::Get();
		// Indicate that the back buffer will be used as a render target.
		CommandContext.TransitionResource(m_RSMFluxBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		CommandContext.TransitionResource(m_RSMNormalBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		CommandContext.TransitionResource(m_RSMPositionBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		CommandContext.TransitionResource(m_RSMDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);

		const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] = { m_RSMFluxBuffer.GetRTV(),m_RSMNormalBuffer.GetRTV(),m_RSMPositionBuffer.GetRTV() };
		CommandContext.SetRenderTargets(3, RTVs, m_RSMDepthBuffer.GetDSV());

		// Record commands.
		CommandContext.ClearColor(m_RSMFluxBuffer);
		CommandContext.ClearColor(m_RSMNormalBuffer);
		CommandContext.ClearColor(m_RSMPositionBuffer);
		CommandContext.ClearDepth(m_RSMDepthBuffer);
		CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		CommandContext.SetPipelineState(m_RSMBufferRenderState->GetPipelineState());

		for (std::shared_ptr<FRenderItem> item : m_AllItems)
		{	
			item->Model->Draw(CommandContext, item.get());
		}

		CommandContext.TransitionResource(m_RSMFluxBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		CommandContext.TransitionResource(m_RSMNormalBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		CommandContext.TransitionResource(m_RSMPositionBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		CommandContext.TransitionResource(m_RSMOutputBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		//CommandContext.Flush(false);
	}

	void ShadingWithRSM(FCommandContext& GfxContext)
	{

		g_CommandListManager.GetComputeQueue().WaitForFenceValue(GfxContext.Flush(true));

		FComputeContext& CommandContext = FComputeContext::Begin(L"Compute Queue");

		CommandContext.SetRootSignature(m_RSMCSSignature);
		CommandContext.SetPipelineState(m_RSMCSPSO);

		Vector3f LightDirInViewSpace = m_Camera.GetViewMatrix().TranslateVector(m_LightDir).Normalize();

		m_RSMBasePassInfo.LightDirInViewSpace = LightDirInViewSpace;
		m_RSMBasePassInfo.LightVP = m_LightVP;
		m_RSMBasePassInfo.InverseCameraView = m_Camera.GetViewMatrix().Inverse();
		m_RSMBasePassInfo.RSMSize = RSM_BUFFER_SIZE;
		memcpy(m_RSMBasePassConstBuf.Map(), &m_RSMBasePassInfo, sizeof(m_RSMBasePassInfo));

		CommandContext.SetDynamicDescriptor(0, 0, m_RSMBasePassCpuHandle);


		CommandContext.SetDynamicDescriptor(1, 0, m_AlbedoBuffer.GetSRV());
		CommandContext.SetDynamicDescriptor(2, 0, m_NormalBuffer.GetSRV());
		CommandContext.SetDynamicDescriptor(3, 0, m_PositionBuffer.GetSRV());
		CommandContext.SetDynamicDescriptor(4, 0, m_RSMFluxBuffer.GetSRV());
		CommandContext.SetDynamicDescriptor(5, 0, m_RSMNormalBuffer.GetSRV());
		CommandContext.SetDynamicDescriptor(6, 0, m_RSMPositionBuffer.GetSRV());
		CommandContext.SetDynamicDescriptor(7, 0, m_RSMOutputBuffer.GetUAV());

		uint32_t GroupCountX = (m_GameDesc.Width + 15) / 16;
		uint32_t GroupCountY = (m_GameDesc.Height + 15) / 16;
		CommandContext.Dispatch(GroupCountX, GroupCountY, 1);
		
		CommandContext.Finish(false);

	}

	void RenderDebug(FCommandContext& CommandContext)
	{
		g_CommandListManager.GetGraphicsQueue().StallForProducer(g_CommandListManager.GetComputeQueue());

		CommandContext.SetRootSignature(m_GeometrySignature);
		CommandContext.SetViewportAndScissor(0, 0, m_GameDesc.Width, m_GameDesc.Height);

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

		CommandContext.SetPipelineState(m_DebugItem.PiplelineInfo->GetPipelineState());
		CommandContext.TransitionResource(m_RSMOutputBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		CommandContext.SetDynamicDescriptor(1, 0, m_RSMOutputBuffer.GetSRV());
		m_DebugItem.Geo->Draw(CommandContext);

		CommandContext.TransitionResource(m_RSMOutputBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_PRESENT);
	}

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override
	{
		m_LastMousePos.x = x;
		m_LastMousePos.y = y;

		::SetCapture(WindowWin32::Get().GetWindowHandle());
	}
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override
	{
		::ReleaseCapture();
	}
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override
	{
		if ((btnState & MK_LBUTTON) != 0)
		{
			// Make each pixel correspond to a quarter of a degree.
			float dx = ConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
			float dy = ConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

			// Update angles based on input to orbit camera around box.
			m_Theta += dx;
			m_Phi += dy;

			// Restrict the angle mPhi.
			m_Phi = Clamp(m_Phi, 0.1f, MATH_PI - 0.1f);

			m_Camera.Rotate(dx, dy);
		}
		else if ((btnState & MK_RBUTTON) != 0)
		{
			// Make each pixel correspond to 0.2 unit in the scene.
			float dx = 0.2f * static_cast<float>(x - m_LastMousePos.x);
			float dy = 0.2f * static_cast<float>(y - m_LastMousePos.y);

			// Update the camera radius based on input.
			m_Radius += dx - dy;

			// Restrict the radius.
			m_Radius = Clamp(m_Radius, 5.0f, 150.0f);
		}

		m_LastMousePos.x = x;
		m_LastMousePos.y = y;
	}

	virtual void OnKeyDown(uint8_t Key)
	{
		const float dt = m_DeltaTime * 20;

		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
			m_SunTheta -= 1.0f * dt;

		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
			m_SunTheta += 1.0f * dt;

		if (GetAsyncKeyState(VK_UP) & 0x8000)
			m_SunPhi -= 1.0f * dt;

		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
			m_SunPhi += 1.0f * dt;

		m_SunPhi = Clamp(m_SunPhi, 0.1f, MATH_PI / 2);
	}

private:

	POINT m_LastMousePos;
	FCamera m_Camera;

	float m_SunTheta = 1.25f * MATH_PI;
	float m_SunPhi = MATH_PI / 4;
	float m_DeltaTime = 0;

	float m_Theta = 1.5f * MATH_PI;
	float m_Phi = MATH_PI / 2 - 0.1f;
	float m_Radius = 50.0f;

	FRootSignature m_RootSignature;
	FRootSignature m_GeometrySignature;
	FRootSignature m_ScreenDebugSignature;
	FRootSignature m_GBufferSignature;

	float m_elapsedTime = 0;
	std::chrono::high_resolution_clock::time_point tStart, tEnd;

	Vector3f m_lightPos;
	Vector3f m_LightDir;
	FMatrix m_LightVP;

	std::shared_ptr< RenderPipelineInfo> m_ColorRenderState;
	std::shared_ptr< RenderPipelineInfo> m_TexutreRenderState;
	std::shared_ptr< RenderPipelineInfo> m_GBufferRenderState;
	std::shared_ptr< RenderPipelineInfo> m_DebugRenderState;
	std::shared_ptr< RenderPipelineInfo> m_RSMBufferRenderState;

	std::vector< std::shared_ptr<FRenderItem> > m_AllItems;

	FColorBuffer m_AlbedoBuffer;
	FColorBuffer m_NormalBuffer;
	FColorBuffer m_PositionBuffer;
	FDepthBuffer m_DepthBuffer;
	FRenderItem m_DebugItem;

	//RSMBuffer
	FColorBuffer m_RSMFluxBuffer;
	FColorBuffer m_RSMNormalBuffer;
	FColorBuffer m_RSMPositionBuffer;
	FDepthBuffer m_RSMDepthBuffer;
	float m_MainRadius = 10;

	//RSM Compute shader
		//computeshader
	FComputePipelineState m_RSMCSPSO;
	std::shared_ptr<FShader> m_RSMShader;
	FRootSignature m_RSMCSSignature;
	FColorBuffer m_RSMOutputBuffer;
	const int m_VPLNum = 32;
	float m_MaxSampleRadius = 25;

	struct
	{
		Vector4f VPLsSampleCoordsAndWeights[32];
		float  MaxSampleRadius = 25;
		float RSMSize;
		int VPLNum = 32;
		Vector3f LightDirInViewSpace;
		FMatrix LightVP;
		FMatrix InverseCameraView;
	} m_RSMBasePassInfo;

	FConstBuffer m_RSMBasePassConstBuf;
	D3D12_CPU_DESCRIPTOR_HANDLE m_RSMBasePassCpuHandle;
};

int main()
{
	ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
	GameDesc Desc;
	Desc.Caption = L"Tutorial08 - Reflective shadow map";
	Tutorial8 tutorial(Desc);
	ApplicationWin32::Get().Run(&tutorial);
	CoUninitialize();
	return 0;
}