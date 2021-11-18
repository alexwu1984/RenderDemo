#include <dxgi1_4.h>
#include <chrono>
#include <iostream>

#include "ApplicationWin32.h"
#include "Game.h"
#include "Common.h"
#include "MathLib.h"
#include "Camera.h"
#include "GLTFModel/GltfModel.h"
#include "GLTFModel/GltfPBRRender.h"
#include "CubeBuffer.h"
#include "SkyBoxPass.h"
#include "SkyBox.h"
#include "GenCubePass.h"
#include "PreIntegratedBRDFPass.h"
#include "CommandListManager.h"
#include "PostProcessing.h"
#include "TemporalEffects.h"
#include "BufferManager.h"

extern FCommandListManager g_CommandListManager;

const int CUBE_MAP_SIZE = 1024;
const int IRRADIANCE_SIZE = 256;
const int PREFILTERED_SIZE = 256;

class Tutorial13 : public FDirectLightGameMode
{

public:
	Tutorial13(const GameDesc& Desc) : FDirectLightGameMode(Desc)
	{
	}

	virtual void OnStartup()
	{
		FDirectLightGameMode::OnUpdate();

		m_Camera = FCamera(Vector3f(2.0f, 0.6f, 0.0f), Vector3f(0.f, 0.3f, 0.f), Vector3f(0.f, 1.f, 0.f));
		m_Camera.SetMouseMoveSpeed(1e-3f);
		m_Camera.SetMouseRotateSpeed(1e-4f);

		const float FovVertical = MATH_PI / 4.f;
		m_Camera.SetPerspectiveParams(FovVertical, (float)GetDesc().Width / GetDesc().Height, 0.5f, 8.f);

		SetupMesh();
		SetupIBL();
	}

	void SetupMesh()
	{
		m_gltfMode = std::make_shared<FGLTFMode>(L"../Resources/Models/Glb/huojian1.glb");
	}

	void SetupIBL()
	{
		m_PreintergrateBRDFPass.Init();

		m_TextureLongLat.LoadFromFile(L"../Resources/HDR/spruit_sunrise_2k.hdr", true);
		m_SkyBox = std::make_shared<FSkyBox>();
		m_SkyPass.Init(m_SkyBox, m_GameDesc.Width, m_GameDesc.Height, L"../Resources/Shaders/EnvironmentShaders.hlsl", "VS_SkyCube", "PS_SkyCube");

		m_CubeBuffer.Create(L"CubeMap", CUBE_MAP_SIZE, CUBE_MAP_SIZE, 0/*full mipmap chain*/, DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_IrradianceCube.Create(L"Irradiance Map", IRRADIANCE_SIZE, IRRADIANCE_SIZE, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);
		m_PrefilteredCube.Create(L"Prefilter Environment Map", PREFILTERED_SIZE, PREFILTERED_SIZE, 0, DXGI_FORMAT_R32G32B32A32_FLOAT);

		m_GenCubePass.Init(m_SkyBox, CUBE_MAP_SIZE, CUBE_MAP_SIZE, L"../Resources/Shaders/EnvironmentShaders.hlsl", "VS_LongLatToCube", "PS_LongLatToCube", FGenCubePass::CubePass_CubeMap);
		m_GenIrradiancePass.Init(m_SkyBox, IRRADIANCE_SIZE, IRRADIANCE_SIZE, L"../Resources/Shaders/EnvironmentShaders.hlsl", "VS_SkyCube", "PS_GenIrradiance", FGenCubePass::CubePass_IrradianceMap);
		m_GenPrefilterEnvMapPass.Init(m_SkyBox, PREFILTERED_SIZE, PREFILTERED_SIZE, L"../Resources/Shaders/EnvironmentShaders.hlsl", "VS_SkyCube", "PS_GenPrefiltered", FGenCubePass::CubePass_PreFilterEnvMap);
		m_PreintegratedBRDF.Create(L"PreintegratedGF", 256, 256, 1, DXGI_FORMAT_R32G32_FLOAT);
		m_PBRRender.InitBase(m_gltfMode, m_GameDesc.Width, m_GameDesc.Height, L"../Resources/Shaders/PBR.hlsl", "VS_PBR", "PS_PBR_GBuffer");
		m_PBRRender.InitIBL(L"../Resources/Shaders/PBR.hlsl", "VS_IBL", "PS_IBL");

		GenerateCubeMap();
		GenerateIrradianceMap();
		GeneratePrefilterEnvironmentMap();
		PreIntegrateBRDF();
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

	virtual void DoRender(FCommandContext& GfxContext)
	{
		m_SkyPass.Render(GfxContext, m_Camera, m_IrradianceCube, true);

		TemporalEffects::ResolveImage(GfxContext, BufferManager::g_SceneColorBuffer);
		PostProcessing::Render(GfxContext);
	}

	virtual void OnUpdate()
	{
		FDirectLightGameMode::OnUpdate();
	}

private:
	std::shared_ptr< FGLTFMode> m_gltfMode;

	FPreIntegratedBRDFPass m_PreintergrateBRDFPass;
	FTexture m_TextureLongLat;
	FColorBuffer m_PreintegratedBRDF;
	FCubeBuffer m_CubeBuffer;
	FCubeBuffer m_IrradianceCube; 
	FCubeBuffer m_PrefilteredCube;
	std::shared_ptr<FSkyBox> m_SkyBox;
	FSkyBoxPass m_SkyPass;
	FGenCubePass m_GenCubePass;
	FGenCubePass m_GenIrradiancePass;
	FGenCubePass m_GenPrefilterEnvMapPass;
	FGlftPBRRender m_PBRRender;
};

int main()
{
	ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
	GameDesc Desc;
	Desc.Caption = L"Tutorial13 - PBR";
	Tutorial13 tutorial(Desc);
	ApplicationWin32::Get().Run(&tutorial);
	CoUninitialize();
	return 0;
}