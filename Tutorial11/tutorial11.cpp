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

		m_GBufferRenderPass.Init(DiffiusePassList, L"../Resources/Shaders/Tutorial10/GBuffer.hlsl",m_GameDesc.Width, m_GameDesc.Height);
		m_ScreenQuadRenderPass.Init(L"../Resources/Shaders/SCreenQuad.hlsl", m_GameDesc.Width, m_GameDesc.Height);
	}

	virtual void OnUpdate()
	{
		FDirectLightGameMode::OnUpdate();
		m_GBufferRenderPass.Update(m_LightInfo.LightDir, m_LightInfo.ViewMatrix, m_LightInfo.ProjectionMatrix, m_Camera);
	}

	virtual void DoRender(FCommandContext& CommandContext)
	{
		m_GBufferRenderPass.Render(CommandContext);
		
		//计算AO
		//m_HBAOPass.Render(CommandContext, m_GBufferRenderPass.GetDepthBuffer());
		//m_HBAOBlurPass.Render(CommandContext, [this](FCommandContext& CommandContext) {
		//	CommandContext.TransitionResource(m_HBAOPass.GetAOBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//	CommandContext.SetDynamicDescriptor(m_HBAOBlurPass.GetSRVRootIndex(), 0, m_HBAOPass.GetAOBuffer().GetSRV());
		//	},
		//	[this](FCommandContext& CommandContext) {
		//		CommandContext.TransitionResource(m_HBAOPass.GetAOBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		//	});


		//合并结果

		//m_ScreenQuadRenderPass.Render(CommandContext, [this](FCommandContext& CommandContext) {
		//	CommandContext.TransitionResource(m_HBAOBlurPass.GetResult(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//	CommandContext.TransitionResource(m_GBufferRenderPass.GetAlbedoBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		//	CommandContext.SetDynamicDescriptor(0, 0, m_HBAOBlurPass.GetResult().GetSRV());
		//	CommandContext.SetDynamicDescriptor(1, 0, m_GBufferRenderPass.GetAlbedoBuffer().GetSRV());
		//	},
		//	[this](FCommandContext& CommandContext) {

		//	});

		m_ScreenQuadRenderPass.Render(CommandContext, [this](FCommandContext& CommandContext) {
			CommandContext.TransitionResource(m_GBufferRenderPass.GetAlbedoBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			CommandContext.SetDynamicDescriptor(0, 0, m_GBufferRenderPass.GetAlbedoBuffer().GetSRV());
			},
			[this](FCommandContext& CommandContext) {

			});
	}

private:
	GBufferRenderPass m_GBufferRenderPass;
	ScreenQuadRenderPass m_ScreenQuadRenderPass;
	SamplePostProcessPass m_HBAOBlurPass;
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