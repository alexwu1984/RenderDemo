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

#include "normalrenderpass.h"
#include "gbufferrenderpass.h"
#include "screenquadrenderpass.h"
#include "ssaopass.h"
#include "SimplePostProcessPass.h"
#include "ssdopass.h"

constexpr int32_t RSM_BUFFER_SIZE = 256;
extern FCommandListManager g_CommandListManager;

class Tutorial9 : public FDirectLightGameMode
{

public:
	Tutorial9(const GameDesc& Desc) : FDirectLightGameMode(Desc)
	{
	}

	virtual void OnStartup()
	{
		FDirectLightGameMode::OnUpdate();

		m_Camera = FCamera(Vector3f(-25.1728, 12.768, -80.1058), Vector3f(0, 0, 0), Vector3f(0, 1, 0));
		std::vector< std::shared_ptr<FRenderItem> > DiffiusePassList;

		std::shared_ptr<FRenderItem> ActorItem = std::make_shared<FRenderItem>();
		ActorItem->Init(L"../Resources/Models/lost_empire/lost_empire.obj");
		ActorItem->Model->SetLightDir(m_LightInfo.LightDir);
		ActorItem->Model->SetLightIntensity(0.5);
		DiffiusePassList.push_back(ActorItem);

		m_GBufferRenderPass.Init(DiffiusePassList, L"../Resources/Shaders/Tutorial09/GBuffer.hlsl",m_GameDesc.Width, m_GameDesc.Height);
		m_ScreenQuadRenderPass.Init(L"../Resources/Shaders/Tutorial09/SCreenQuad.hlsl", m_GameDesc.Width, m_GameDesc.Height);
		m_SSAOPass.Init(m_GameDesc.Width, m_GameDesc.Height);
		m_AOBlurPass.Init( L"../Resources/Shaders/Tutorial09/SSAOBlurPass.hlsl", m_GameDesc.Width, m_GameDesc.Height);
		m_DOBlurPass.Init( L"../Resources/Shaders/Tutorial09/SSDOBlurPass.hlsl", m_GameDesc.Width, m_GameDesc.Height);
		m_SSDOPass.Init(m_GameDesc.Width, m_GameDesc.Height);
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
		m_SSAOPass.Render(CommandContext, m_GBufferRenderPass.GetPositionBuffer(), m_GBufferRenderPass.GetNormalBuffer());
		m_AOBlurPass.Render(CommandContext, [this](FCommandContext& CommandContext) {
			CommandContext.TransitionResource(m_SSAOPass.GetAOBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			CommandContext.SetDynamicDescriptor(m_AOBlurPass.GetSRVRootIndex(), 0, m_SSAOPass.GetAOBuffer().GetSRV());
			}, 
			[this](FCommandContext& CommandContext) {
			CommandContext.TransitionResource(m_SSAOPass.GetAOBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		});

		//计算DO
		m_SSDOPass.Render(CommandContext, m_GBufferRenderPass.GetPositionBuffer(), m_GBufferRenderPass.GetNormalBuffer(), m_GBufferRenderPass.GetAlbedoBuffer());
		m_DOBlurPass.Render(CommandContext, [this](FCommandContext& CommandContext) {
			CommandContext.TransitionResource(m_SSDOPass.GetAOBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			CommandContext.SetDynamicDescriptor(m_AOBlurPass.GetSRVRootIndex(), 0, m_SSDOPass.GetAOBuffer().GetSRV());
			},
			[this](FCommandContext& CommandContext) {
				CommandContext.TransitionResource(m_SSDOPass.GetAOBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			});

		//合并结果

		m_ScreenQuadRenderPass.Render(CommandContext, [this](FCommandContext& CommandContext) {
			CommandContext.TransitionResource(m_AOBlurPass.GetResult(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			CommandContext.TransitionResource(m_DOBlurPass.GetResult(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			CommandContext.TransitionResource(m_GBufferRenderPass.GetAlbedoBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			CommandContext.SetDynamicDescriptor(0, 0, m_AOBlurPass.GetResult().GetSRV());
			CommandContext.SetDynamicDescriptor(1, 0, m_DOBlurPass.GetResult().GetSRV());
			CommandContext.SetDynamicDescriptor(2, 0, m_GBufferRenderPass.GetAlbedoBuffer().GetSRV());
			}, 
			[this](FCommandContext& CommandContext) {

		});
	}

private:
	NormalRenderPass m_NormalRenderPass;
	GBufferRenderPass m_GBufferRenderPass;
	ScreenQuadRenderPass m_ScreenQuadRenderPass;
	SSAOPass m_SSAOPass;
	FSimplePostProcessPass m_AOBlurPass;
	FSimplePostProcessPass m_DOBlurPass;
	SSDOPass m_SSDOPass;
};

int main()
{
	ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
	GameDesc Desc;
	Desc.Caption = L"Tutorial09 - Approximating Dynamic Global Illumination in Image Space";
	Tutorial9 tutorial(Desc);
	ApplicationWin32::Get().Run(&tutorial);
	CoUninitialize();
	return 0;
}