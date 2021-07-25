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

#include <dxgi1_4.h>
#include <chrono>
#include <iostream>

#include "gbufferrenderpass.h"
#include "screenquadrenderpass.h"
#include "samplepostprocesspass.h"
#include "ScreenSpaceRayTracingPass.h"

constexpr int32_t RSM_BUFFER_SIZE = 256;
extern FCommandListManager g_CommandListManager;

class Tutorial11 : public FDirectLightGameMode
{

public:
	Tutorial11(const GameDesc& Desc) : FDirectLightGameMode(Desc)
	{
	}

	virtual void OnStartup()
	{
		FDirectLightGameMode::OnUpdate();

		m_Camera = FCamera(Vector3f(0, -2.03285, -3.00298), Vector3f(0.f, 0.0f, 0.f), Vector3f(0.f, 1.f, 0.f));
		std::vector< std::shared_ptr<FRenderItem> > DiffiusePassList;

		std::shared_ptr<FRenderItem> ActorItem = std::make_shared<FRenderItem>();
		ActorItem->Init(L"../Resources/Models/VioletSponza/SponzaPBR.obj");
		ActorItem->Model->SetLightDir(m_LightInfo.LightDir);
		ActorItem->Model->SetLightIntensity(0.5);
		DiffiusePassList.push_back(ActorItem);

		m_GBufferRenderPass.Init(DiffiusePassList, L"../Resources/Shaders/Tutorial11/GBuffer.hlsl",m_GameDesc.Width, m_GameDesc.Height);
		m_SSRPass.Init(DiffiusePassList, L"../Resources/Shaders/Tutorial11/ScreenSpaceRayTracing.hlsl", m_GameDesc.Width, m_GameDesc.Height);
		m_ScreenQuadRenderPass.Init(L"../Resources/Shaders/SCreenQuad.hlsl", m_GameDesc.Width, m_GameDesc.Height);
		
	}

	virtual void OnUpdate()
	{
		FDirectLightGameMode::OnUpdate();
		m_GBufferRenderPass.Update(m_LightInfo.LightDir, m_LightInfo.ViewMatrix, m_LightInfo.ProjectionMatrix, m_Camera);
		m_SSRPass.Update(m_Camera);
	}

	virtual void DoRender(FCommandContext& CommandContext)
	{
		m_GBufferRenderPass.Render(CommandContext);
		m_SSRPass.Render(CommandContext, m_GBufferRenderPass.GetDepthBuffer(), m_GBufferRenderPass.GetAlbedoBuffer());

		m_ScreenQuadRenderPass.Render(CommandContext, [this](FCommandContext& CommandContext) {
			CommandContext.TransitionResource(m_SSRPass.GetAlbedoBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			CommandContext.SetDynamicDescriptor(0, 0, m_SSRPass.GetAlbedoBuffer().GetSRV());
			},
			[this](FCommandContext& CommandContext) {

			});
	}

private:
	GBufferRenderPass m_GBufferRenderPass;
	ScreenQuadRenderPass m_ScreenQuadRenderPass;
	ScreenSpaceRayTracingPass m_SSRPass;
};

int main()
{
	ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
	GameDesc Desc;
	Desc.Caption = L"Tutorial11 - Efficient GPU Screen-Space Ray Tracing";
	Tutorial11 tutorial(Desc);
	ApplicationWin32::Get().Run(&tutorial);
	CoUninitialize();
	return 0;
}