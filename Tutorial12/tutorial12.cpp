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
#include "GenCubePass.h"
#include "CubeMapCross.h"
#include "PreIntegratedBRDFPass.h"
#include "pbrrenderpass.h"

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
	SM_PreintegratedBRDF,
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

		m_Camera = FCamera(Vector3f(1.5f, 1.f, 0.f), Vector3f(0.f, 0.3f, 0.f), Vector3f(0.f, 1.f, 0.f));
		m_Camera.SetMouseMoveSpeed(1e-3f);
		m_Camera.SetMouseRotateSpeed(1e-4f);

		const float FovVertical = MATH_PI / 4.f;
		m_Camera.SetPerspectiveParams(FovVertical, (float)GetDesc().Width / GetDesc().Height, 0.1f, 100.f);

		SetupMesh();
		GenerateCubeMap();
		GenerateIrradianceMap();
		GeneratePrefilterEnvironmentMap();
		PreIntegrateBRDF();
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
			ImGui::RadioButton("PreintegratedBRDF", &m_ShowMode, SM_PreintegratedBRDF);
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
		m_PBRRenderPass.Update(m_Camera);

		if (m_RotateMesh)
		{
			m_RotateY += m_Delta * 0.0005f;
			m_RotateY = fmodf(m_RotateY, MATH_2PI);
			m_PBRRenderPass.Rotate(m_RotateY);
		}
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
		case SM_SkyBox:
			SkyPass(GfxContext, m_CubeBuffer);
			break;
		case SM_CubeMapCross:
			m_CubeMapCrossDebug.ShowCubeMapDebugView(GfxContext, m_CubeBuffer, 1.0, m_MipLevel);
			break;
		case SM_Irradiance:
			m_CubeMapCrossDebug.ShowCubeMapDebugView(GfxContext, m_IrradianceCube, 1.0, m_MipLevel);
			break;
		case SM_Prefiltered:
			m_CubeMapCrossDebug.ShowCubeMapDebugView(GfxContext, m_PrefilteredCube, 1.0, m_MipLevel);
			break;
		case SM_PreintegratedBRDF:
			ShowTexture2D(GfxContext, m_PreintegratedBRDF);
			break;
		case SM_PBR:
			SkyPass(GfxContext, m_CubeBuffer);
			RenderMesh(GfxContext);
			
			break;
		};

		OnGUI(GfxContext);
	}

	void SetupMesh()
	{
		std::vector< std::shared_ptr<FRenderItem> > DiffiusePassList;

		std::shared_ptr<FRenderItem> ActorItem = std::make_shared<FRenderItem>();
		ActorItem->Init(L"../Resources/Models/harley/harley.obj",true,false);
		ActorItem->Model->SetLightDir(m_LightInfo.LightDir);
		ActorItem->Model->SetLightIntensity(0.5);
		DiffiusePassList.push_back(ActorItem);

		m_LongLatPass.Init();
		m_PreintergrateBRDFPass.Init();

		m_TextureLongLat.LoadFromFile(L"../Resources/HDR/spruit_sunrise_2k.hdr",true);
		m_SkyBox = std::make_shared<FSkyBox>();
		m_SkyPass.Init(m_SkyBox, m_GameDesc.Width, m_GameDesc.Height, L"../Resources/Shaders/EnvironmentShaders.hlsl", "VS_SkyCube", "PS_SkyCube");
		m_CubeMapCross = std::make_shared<FCubeMapCross>();

		m_CubeBuffer.Create(L"CubeMap", CUBE_MAP_SIZE, CUBE_MAP_SIZE, 0/*full mipmap chain*/, DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_IrradianceCube.Create(L"Irradiance Map", IRRADIANCE_SIZE, IRRADIANCE_SIZE, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
		m_PrefilteredCube.Create(L"Prefilter Environment Map", PREFILTERED_SIZE, PREFILTERED_SIZE, 0, DXGI_FORMAT_R16G16B16A16_FLOAT);

		m_GenCubePass.Init(m_SkyBox, CUBE_MAP_SIZE, CUBE_MAP_SIZE,L"../Resources/Shaders/EnvironmentShaders.hlsl", "VS_LongLatToCube", "PS_LongLatToCube",GenCubePass::CubePass_CubeMap);
		m_GenIrradiancePass.Init(m_SkyBox, IRRADIANCE_SIZE, IRRADIANCE_SIZE, L"../Resources/Shaders/EnvironmentShaders.hlsl", "VS_SkyCube", "PS_GenIrradiance",GenCubePass::CubePass_IrradianceMap);
		m_GenPrefilterEnvMapPass.Init(m_SkyBox, PREFILTERED_SIZE, PREFILTERED_SIZE, L"../Resources/Shaders/EnvironmentShaders.hlsl", "VS_SkyCube", "PS_GenPrefiltered", GenCubePass::CubePass_PreFilterEnvMap);
		m_CubeMapCrossDebug.Init(m_CubeMapCross, m_GameDesc.Width, m_GameDesc.Height, L"../Resources/Shaders/EnvironmentShaders.hlsl", "VS_CubeMapCross", "PS_CubeMapCross");

		m_PreintegratedBRDF.Create(L"PreintegratedGF", 128, 32, 1, DXGI_FORMAT_R32G32_FLOAT);
		m_PBRRenderPass.Init(DiffiusePassList, m_GameDesc.Width, m_GameDesc.Height, L"../Resources/Shaders/PBR.hlsl", "VS_PBR", "PS_PBR");
		
	}

	void ShowTexture2D(FCommandContext& GfxContext, FTexture& Texture2D)
	{
		float AspectRatio = Texture2D.GetWidth() * 1.f / Texture2D.GetHeight();
		int Width = std::min(m_GameDesc.Width, Texture2D.GetWidth());
		int Height = std::min(m_GameDesc.Height, Texture2D.GetHeight());
		Width = std::min(Width, static_cast<int>(Height * AspectRatio));
		Height = std::min(Height, static_cast<int>(Width / AspectRatio));
		m_LongLatPass.SetViewportAndScissor((m_GameDesc.Width - Width) / 2, (m_GameDesc.Height - Height) / 2, Width, Height);
		m_LongLatPass.ShowTexture2D(GfxContext, Texture2D.GetSRV());
	}

	void ShowTexture2D(FCommandContext& GfxContext, FColorBuffer& Texture2D)
	{
		GfxContext.TransitionResource(Texture2D, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		float AspectRatio = Texture2D.GetWidth() * 1.f / Texture2D.GetHeight();
		int Width = std::min((uint32_t)m_GameDesc.Width, Texture2D.GetWidth());
		int Height = std::min((uint32_t)m_GameDesc.Height, Texture2D.GetHeight());
		Width = std::min(Width, static_cast<int>(Height * AspectRatio));
		Height = std::min(Height, static_cast<int>(Width / AspectRatio));
		m_LongLatPass.SetViewportAndScissor((m_GameDesc.Width - Width) / 2, (m_GameDesc.Height - Height) / 2, Width, Height);
		m_LongLatPass.ShowTexture2D(GfxContext, Texture2D.GetSRV());
	}

	void GenerateCubeMap()
	{
		m_GenCubePass.GenerateCubeMap(m_CubeBuffer, m_TextureLongLat);
	}

	void GenerateIrradianceMap()
	{
		m_GenIrradiancePass.GenerateIrradianceMap(m_CubeBuffer, m_IrradianceCube, 10);
	}

	void GeneratePrefilterEnvironmentMap()
	{
		m_GenPrefilterEnvMapPass.GeneratePrefilteredMap(m_CubeBuffer, m_PrefilteredCube);
	}

	void PreIntegrateBRDF()
	{
		m_PreintergrateBRDFPass.IntegrateBRDF(m_PreintegratedBRDF);
	}

	void SkyPass(FCommandContext& GfxContext,FCubeBuffer& CubeBuffer)
	{
		m_SkyPass.Render(GfxContext,m_Camera, CubeBuffer);
	}

	void RenderMesh(FCommandContext& GfxContext)
	{
		m_PBRRenderPass.Render(GfxContext, m_Camera, m_IrradianceCube, m_PrefilteredCube, m_PreintegratedBRDF);
	}

private:

	FTexture m_TextureLongLat;
	FColorBuffer m_PreintegratedBRDF;
	FCubeBuffer m_CubeBuffer, m_IrradianceCube, m_PrefilteredCube;
	int m_ShowMode = SM_PBR;
	Vector3f m_ClearColor = Vector3f(0.2f);
	float m_Exposure = 1.f;
	int m_MipLevel = 0;
	int m_NumSamplesPerDir = 10;
	bool m_RotateMesh = false;
	float m_RotateY = 0.f;
	Show2DTexturePass m_LongLatPass;
	PreIntegratedBRDFPass m_PreintergrateBRDFPass;
	PBRRenderPass m_PBRRenderPass;

	SkyBoxPass m_SkyPass;
	SkyBoxPass m_CubeMapCrossDebug;
	GenCubePass m_GenCubePass;
	GenCubePass m_GenIrradiancePass;
	GenCubePass m_GenPrefilterEnvMapPass;
	std::shared_ptr< FSkyBox> m_SkyBox;
	std::shared_ptr< FCubeMapCross > m_CubeMapCross;
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