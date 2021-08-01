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
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "Geometry.h"
#include "ImguiManager.h"

#include <dxgi1_4.h>
#include <chrono>
#include <iostream>

#include "pbrrenderpass.h"
#include "Show2DTexturePass.h"
#include "CubeBuffer.h"
#include "SkyBoxPass.h"
#include "SkyBox.h"

extern FCommandListManager g_CommandListManager;

const int CUBE_MAP_SIZE = 1024;
const int IRRADIANCE_SIZE = 256;
const int PREFILTERED_SIZE = 256;
const bool CUBEMAP_DEBUG_VIEW = true;

EVSConstants g_EVSConstants;
EPSConstants g_EPSConstants;

enum EShowMode
{
	SM_LongLat,
	SM_SkyBox,
	SM_CubeMapCross,
	SM_Irradiance,
	SM_Prefiltered,
	//SM_SphericalHarmonics,
	SM_PreintegratedGF,
	SM_PBR,
};

class Tutorial12 : public FDirectLightGameMode
{

public:
	Tutorial12(const GameDesc& Desc) : FDirectLightGameMode(Desc)
	{
	}

	virtual void OnStartup()
	{
		FDirectLightGameMode::OnUpdate();

		m_Camera = FCamera(Vector3f(0, -2.03285, -3.00298), Vector3f(-0.305803, 0.190466, -0.932849), Vector3f(0.f, 1.f, 0.f));
		std::vector< std::shared_ptr<FRenderItem> > DiffiusePassList;

		std::shared_ptr<FRenderItem> ActorItem = std::make_shared<FRenderItem>();
		ActorItem->Init(L"../Resources/Models/harley/harley.obj");
		ActorItem->Model->SetLightDir(m_LightInfo.LightDir);
		ActorItem->Model->SetLightIntensity(0.5);
		DiffiusePassList.push_back(ActorItem);

		m_LongLatPass.Init();

		SetupMesh();

		//m_GBufferRenderPass.Init(DiffiusePassList, L"../Resources/Shaders/Tutorial11/GBuffer.hlsl",m_GameDesc.Width, m_GameDesc.Height);
		//m_SSRPass.Init(DiffiusePassList, L"../Resources/Shaders/Tutorial11/ScreenSpaceRayTracing.hlsl", m_GameDesc.Width, m_GameDesc.Height);
		//m_ScreenQuadRenderPass.Init(L"../Resources/Shaders/SCreenQuad.hlsl", m_GameDesc.Width, m_GameDesc.Height);
		
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
			ImGui::ColorEdit3("Clear Color", &m_ClearColor.x);

			ImGui::Separator();

			ImGui::BeginGroup();
			ImGui::Text("Show Mode");
			ImGui::Indent(20);
			ImGui::RadioButton("Long-Lat View", &m_ShowMode, SM_LongLat);
			ImGui::RadioButton("Cube Box", &m_ShowMode, SM_SkyBox);
			ImGui::RadioButton("Cube Cross", &m_ShowMode, SM_CubeMapCross);
			ImGui::RadioButton("Irradiance", &m_ShowMode, SM_Irradiance);
			ImGui::RadioButton("Prefiltered", &m_ShowMode, SM_Prefiltered);
			//ImGui::RadioButton("SphericalHarmonics", &m_ShowMode, SM_SphericalHarmonics);
			ImGui::RadioButton("PreintegratedGF", &m_ShowMode, SM_PreintegratedGF);
			ImGui::RadioButton("PBR Mesh", &m_ShowMode, SM_PBR);
			ImGui::EndGroup();

			if (m_ShowMode == SM_CubeMapCross)
			{
				ImGui::SliderInt("Mip Level", &m_MipLevel, 0, m_CubeBuffer.GetNumMips() - 1);
			}
			else if (m_ShowMode == SM_Prefiltered)
			{
				ImGui::SliderInt("Mip Level", &m_MipLevel, 0, m_PrefilteredCube.GetNumMips() - 1);
			}
			//else if (m_ShowMode == SM_SphericalHarmonics)
			//{
			//	ImGui::SliderInt("SH Degree", &m_SHDegree, 1, 4);
			//}
			else if (m_ShowMode == SM_PBR)
			{
				ImGui::Checkbox("Rotate Mesh", &m_RotateMesh);
				ImGui::SameLine();
				ImGui::Text("%.3f", m_RotateY);
			}

			ImGui::Separator();

		}
		ImGui::End();

		ImguiManager::Get().Render(CommandContext, RenderWindow::Get());
	}

	virtual void OnUpdate()
	{
		FDirectLightGameMode::OnUpdate();

	}

	virtual void DoRender(FCommandContext& GfxContext)
	{

		RenderWindow& renderWindow = RenderWindow::Get();
		FColorBuffer& BackBuffer = renderWindow.GetBackBuffer();
		GfxContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET,true);
		GfxContext.ClearColor(BackBuffer);

		switch (m_ShowMode)
		{
		case SM_LongLat:
			ShowTexture2D(GfxContext, m_TextureLongLat);
			break;
		};

		OnGUI(GfxContext);
	}

	void SetupMesh()
	{
		m_TextureLongLat.LoadFromFile(L"../Resources/HDR/spruit_sunrise_2k.hdr",true);
		m_SkyBox = std::make_shared<FSkyBox>();
		m_SkeyPass.Init(m_SkyBox, m_GameDesc.Width, m_GameDesc.Height);

		m_CubeBuffer.Create(L"CubeMap", CUBE_MAP_SIZE, CUBE_MAP_SIZE, 0/*full mipmap chain*/, DXGI_FORMAT_R16G16B16A16_FLOAT);
	}

	void ShowTexture2D(FCommandContext& GfxContext, FTexture& Texture2D)
	{
		float AspectRatio = Texture2D.GetWidth() * 1.f / Texture2D.GetHeight();
		int Width = std::min(m_GameDesc.Width, Texture2D.GetWidth());
		int Height = std::min(m_GameDesc.Height, Texture2D.GetHeight());
		Width = std::min(Width, static_cast<int>(Height * AspectRatio));
		Height = std::min(Height, static_cast<int>(Width / AspectRatio));
		m_LongLatPass.SetViewportAndScissor((m_GameDesc.Width - Width) / 2, (m_GameDesc.Height - Height) / 2, Width, Height);
		m_LongLatPass.Render(GfxContext, Texture2D);
	}

	void GenerateCubeMap()
	{

	}

private:

	FTexture m_TextureLongLat;
	FCubeBuffer m_CubeBuffer, m_IrradianceCube, m_PrefilteredCube;
	int m_ShowMode = SM_PBR;
	Vector3f m_ClearColor = Vector3f(0.2f);
	float m_Exposure = 1.f;
	int m_MipLevel = 0;
	int m_NumSamplesPerDir = 10;
	bool m_RotateMesh = false;
	float m_RotateY = 0.f;
	Show2DTexturePass m_LongLatPass;
	SkyBoxPass m_SkeyPass;
	std::shared_ptr< FSkyBox> m_SkyBox;
};

int main()
{
	ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
	GameDesc Desc;
	Desc.Caption = L"Tutorial12 - PBR";
	Tutorial12 tutorial(Desc);
	ApplicationWin32::Get().Run(&tutorial);
	CoUninitialize();
	return 0;
}