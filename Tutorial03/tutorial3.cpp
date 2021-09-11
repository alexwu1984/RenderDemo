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

#include <d3d12.h>
#include <dxgi1_4.h>
#include <chrono>
#include <iostream>
#include <random>

#include "BulletPhysics.h"
#include "BulletRenderItem.h"
#include "ImguiManager.h"

extern FCommandListManager g_CommandListManager;

extern void ConvertFMatrix(const FMatrix& in, DirectX::XMMATRIX& out);



class Tutorial3 : public FGame
{
public:
	Tutorial3(const GameDesc& Desc) : FGame(Desc), m_Camera(Vector3f(0.f, 0.f, -5.f), Vector3f(0.f, 0.0f, 0.f), Vector3f(0.f, 1.f, 0.f))
	{
	}

	void OnStartup()
	{
		SetupRootSignature();
		SetupShaders();
		SetupMesh();
		SetupPipelineState();
	}

	void OnUpdate()
	{

	}

	void OnGUI(FCommandContext& CommandContext)
	{

		static bool ShowConfig = true;
		if (!ShowConfig)
			return;

		ImguiManager::Get().NewFrame();

		ImGui::SetNextWindowPos(ImVec2(1, 1));
		ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
		if (ImGui::Begin("Config", &ShowConfig, ImGuiWindowFlags_AlwaysAutoResize))
		{
			//ImGui::ColorEdit3("Clear Color", &m_ClearColor.x);
			//ImGui::SliderFloat("Exposure", &m_Exposure, 0.f, 10.f, "%.1f");

			ImGui::Separator();

			ImGui::SliderInt("Box Count", &m_MaxBoxCount, 5, 80);

			//ImGui::BeginGroup();
			//ImGui::Text("Show Mode");
			//ImGui::Indent(20);
			//ImGui::RadioButton("Long-Lat View", &m_ShowMode, SM_LongLat);
			//ImGui::RadioButton("Cube Box", &m_ShowMode, SM_SkyBox);
			//ImGui::RadioButton("Cube Cross", &m_ShowMode, SM_CubeMapCross);
			//ImGui::RadioButton("Irradiance", &m_ShowMode, SM_Irradiance);
			//ImGui::RadioButton("Prefiltered", &m_ShowMode, SM_Prefiltered);
			//ImGui::RadioButton("SphericalHarmonics", &m_ShowMode, SM_SphericalHarmonics);
			//ImGui::RadioButton("PreintegratedGF", &m_ShowMode, SM_PreintegratedGF);
			//ImGui::RadioButton("PBR Mesh", &m_ShowMode, SM_PBR);
			//ImGui::EndGroup();

			//if (m_ShowMode == SM_CubeMapCross)
			//{
			//	ImGui::SliderInt("Mip Level", &m_MipLevel, 0, m_CubeBuffer.GetNumMips() - 1);
			//}
			//else if (m_ShowMode == SM_Prefiltered)
			//{
			//	ImGui::SliderInt("Mip Level", &m_MipLevel, 0, m_PrefilteredCube.GetNumMips() - 1);
			//}
			//else if (m_ShowMode == SM_SphericalHarmonics)
			//{
			//	ImGui::SliderInt("SH Degree", &m_SHDegree, 1, 4);
			//}
			//else if (m_ShowMode == SM_PBR)
			//{
			//	ImGui::Checkbox("StaticSceneTAA", &m_bStaticSceneTAA);

			//	TemporalEffects::g_EnableTAA = m_bStaticSceneTAA;

			//	ImGui::Checkbox("SHDiffuse", &m_bSHDiffuse);
			//	ImGui::Checkbox("Rotate Mesh", &m_RotateMesh);
			//	ImGui::SameLine();
			//	ImGui::Text("%.3f", m_RotateY);
			//}

			ImGui::Separator();

			//static bool ShowDemo = false;
			//ImGui::Checkbox("Show Demo", &ShowDemo);
			//if (ShowDemo)
			//	ImGui::ShowDemoWindow(&ShowDemo);

		}
		ImGui::End();

		ImguiManager::Get().Render(CommandContext, RenderWindow::Get());
	}

	virtual void OnShutdown()
	{
		m_BulletPhysic.UnInit();
	}
	
	void OnRender()
	{
		
		// Frame limit set to 60 fps
		tEnd = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::milli>(tEnd - tStart).count();
		if (time < (1000.0f / 60.0f))
		{
			return;
		}

		FCommandContext& CommandContext = FCommandContext::Begin();

		tStart = std::chrono::high_resolution_clock::now();

		// Update Uniforms
		m_elapsedTime += 0.001f * time;
		m_elapsedTime = fmodf(m_elapsedTime, 6.283185307179586f);
		UpdateStatus();

		m_BulletPhysic.UpdateScene(0.001f * time);

		FillCommandLists(CommandContext);
		
		CommandContext.Finish(true);

		RenderWindow::Get().Present();	
	}

private:
	void SetupRootSignature()
	{
		m_RootSignature.Reset(1, 0);
		m_RootSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
		m_RootSignature.Finalize(L"Tutorial3", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	}

	void SetupMesh()
	{
		m_BulletPhysic.Init();
		m_BulletPhysic.CreateGroundPlane();

		const float FovVertical = MATH_PI / 4.f;

		m_uboVS.viewMatrix = m_Camera.GetViewMatrix();
		m_uboVS.projectionMatrix = FMatrix::MatrixPerspectiveFovLH(FovVertical, (float)GetDesc().Width / GetDesc().Height, 0.1f, 100.f);

		for (int index  = 0; index < m_MaxBoxCount; ++index)
		{
			CreateBox();
		}

		DirectX::XMMATRIX dxMat;
		ConvertFMatrix(m_uboVS.projectionMatrix, dxMat);
		DirectX::BoundingFrustum::CreateFromMatrix(m_Frustum, dxMat);

	}

	void SetupShaders()
	{
		m_vertexShader = D3D12RHI::Get().CreateShader(L"../Resources/Shaders/simplebox.hlsl", "vs_main", "vs_5_1");
		m_pixelShader = D3D12RHI::Get().CreateShader(L"../Resources/Shaders/simplebox.hlsl", "ps_main", "ps_5_1");
	}

	void SetupPipelineState()
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
		MeshLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		MeshLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		MeshLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

		m_PipelineState.SetRootSignature(m_RootSignature);
		m_PipelineState.SetRasterizerState(FPipelineState::RasterizerDefault);
		m_PipelineState.SetBlendState(FPipelineState::BlendDisable);
		m_PipelineState.SetDepthStencilState(FPipelineState::DepthStateReadWrite);
		m_PipelineState.SetInputLayout(MeshLayout.size(), MeshLayout.data());
		m_PipelineState.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		m_PipelineState.SetRenderTargetFormats(1, &RenderWindow::Get().GetColorFormat(), RenderWindow::Get().GetDepthFormat());
		m_PipelineState.SetVertexShader(CD3DX12_SHADER_BYTECODE(m_vertexShader.Get()));
		m_PipelineState.SetPixelShader(CD3DX12_SHADER_BYTECODE(m_pixelShader.Get()));
		m_PipelineState.Finalize();
	}

	void FillCommandLists(FCommandContext& CommandContext)
	{
		// Set necessary state.
		CommandContext.SetRootSignature(m_RootSignature);
		CommandContext.SetPipelineState(m_PipelineState);
		CommandContext.SetViewportAndScissor(0, 0, m_GameDesc.Width, m_GameDesc.Height);

		RenderWindow& renderWindow = RenderWindow::Get();
		FColorBuffer& BackBuffer = renderWindow.GetBackBuffer();
		FDepthBuffer& DepthBuffer = renderWindow.GetDepthBuffer();
		// Indicate that the back buffer will be used as a render target.
		CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		CommandContext.TransitionResource(DepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
		CommandContext.SetRenderTargets(1, &BackBuffer.GetRTV(), DepthBuffer.GetDSV());

		// Record commands.
		BackBuffer.SetClearColor(m_ClearColor);
		CommandContext.ClearColor(BackBuffer);
		CommandContext.ClearDepth(DepthBuffer);
		CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (auto& item : m_RenderList)
		{
			item->UpdateState(m_Frustum, m_Camera);

			BulletRenderItem::BasePassInfoWrapper& wapper = item->PassInfo;

			const FMatrix& fMat = item->Geo.GetModelMatrix();

			wapper.BasePassInfo.modelMatrix = fMat;
			memcpy(wapper.BasePassConstBuf.Map(), &wapper.BasePassInfo, sizeof(wapper.BasePassInfo));
			CommandContext.SetDynamicDescriptor(item->CBVRootIndex, 0, wapper.BasePassCpuHandle);
			item->Geo.Draw(CommandContext);
		}

		OnGUI(CommandContext);

		CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_PRESENT, true);
	}

	void UpdateStatus()
	{
		//删除物理系统元素
		m_BulletPhysic.RemoveItems();
		for (auto itRemove = m_RenderList.begin(); itRemove != m_RenderList.end(); )
		{
			if ((*itRemove)->IsDelete)
			{
				m_ReUseList.push_back(*itRemove);
				itRemove = m_RenderList.erase(itRemove);
				
			}
			else
			{
				++itRemove;
			}
		}

		//重用DX元素，物理系统的元素不能重用，试过保留，但是没有效果，需要重新生成
		while (!m_ReUseList.empty())
		{
			auto item = m_ReUseList.front();
			if (m_RenderList.size() < m_MaxBoxCount)
			{
				item->IsDelete = false;
				item->RunState = BulletRenderItem::UnKnown;
				m_BulletPhysic.CreateDynamicObject(item.get());
				m_RenderList.push_back(item);
			}

			m_ReUseList.pop_front();
		}

		while (m_RenderList.size() < m_MaxBoxCount)
		{
			CreateBox();
		}
	}

	void CreateBox()
	{

		std::shared_ptr<BulletRenderItem> box = std::make_shared<BulletRenderItem>();
		box->Init();
		box->Geo.CreateCube(0.5, 0.2, 0.5);
		box->Geo.SetPosition(Vector3f(0, 0, 0));

		const auto& BoundBox = box->Geo.GetBoundBox();

		box->Box.Center = DirectX::XMFLOAT3(BoundBox.Center.x, BoundBox.Center.y, BoundBox.Center.z);
		box->Box.Extents = DirectX::XMFLOAT3(BoundBox.Extents.x, BoundBox.Extents.y, BoundBox.Extents.z);


		m_RenderList.push_back(box);
		m_BulletPhysic.CreateDynamicObject(box.get());
		box->PassInfo.BasePassInfo.viewMatrix = m_uboVS.viewMatrix;
		box->PassInfo.BasePassInfo.projectionMatrix = m_uboVS.projectionMatrix;
	}


private:
	struct
	{
		FMatrix projectionMatrix;
		FMatrix viewMatrix;
	} m_uboVS;

	FRootSignature m_RootSignature;
	FGraphicsPipelineState m_PipelineState;

	ComPtr<ID3DBlob> m_vertexShader;
	ComPtr<ID3DBlob> m_pixelShader;

	DirectX::BoundingFrustum m_Frustum;

	float m_elapsedTime = 0;
	std::chrono::high_resolution_clock::time_point tStart, tEnd;

	BulletPhysic m_BulletPhysic;

	std::list<std::shared_ptr<BulletRenderItem>> m_RenderList;
	std::list<std::shared_ptr<BulletRenderItem>> m_ReUseList;

	int m_MaxBoxCount = 60;

	FCamera m_Camera;
	Vector3f m_ClearColor = Vector3f(0.2);
};

int main()
{
	GameDesc Desc;
	Desc.Caption = L"Tutorial 3 - Draw Cube";
	Tutorial3 tutorial(Desc);
	ApplicationWin32::Get().Run(&tutorial);
	return 0;
}