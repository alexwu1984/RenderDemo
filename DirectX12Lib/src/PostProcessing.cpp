#include "PostProcessing.h"
#include "Common.h"
#include "RootSignature.h"
#include "CommandContext.h"
#include "WindowWin32.h"
#include "PipelineState.h"
#include "D3D12RHI.h"
#include "ColorBuffer.h"
#include "SamplerManager.h"
#include "BufferManager.h"
#include "RenderWindow.h"
#include "CommandListManager.h"
#include "Texture.h"
#include "Camera.h"
#include "CubeBuffer.h"
#include "UserMarkers.h"
#include "TemporalEffects.h"
#include "MotionBlur.h"

using namespace BufferManager;
extern FCommandListManager g_CommandListManager;

namespace PostProcessing
{
	bool g_EnableBloom = false;

	float g_BloomIntensity = 1.f;
	float g_BloomThreshold = 1.f;
	bool g_EnableSSR = false;
	bool g_UseHiZ = true;
	bool g_UseMinMaxZ = false;
	float g_Thickness = 0.08f;
	float g_WorldThickness = 0.06f;
	float g_CompareTolerance = 0.027f;
	int g_NumRays = 2;

	FRootSignature m_CSSignature;
	FRootSignature m_PostProcessSignature;

	FComputePipelineState m_CSUpSamplePSO;
	FComputePipelineState m_CSDownSamplePSO;
	FComputePipelineState m_CSExtractBloomPSO;
	FComputePipelineState m_CSBuildHZBPSO;
	FGraphicsPipelineState m_ToneMapWithBloomPSO;
	FGraphicsPipelineState m_SSRPSO;

	ComPtr<ID3DBlob> m_UpSampleCS;
	ComPtr<ID3DBlob> m_DownSampleCS;
	ComPtr<ID3DBlob> m_ExtractBloomCS;
	ComPtr<ID3DBlob> m_BuildHZBCS;

	ComPtr<ID3DBlob> m_ScreenQuadVS;
	ComPtr<ID3DBlob> m_ToneMapWithBloomPS;
	ComPtr<ID3DBlob> m_SSRPS;

	FColorBuffer g_BloomBuffers[5];
	FColorBuffer g_HiZBuffer;

	FTexture m_BlackTexture;

	__declspec(align(16)) struct
	{
		float BloomIntensity;
		float BloomThreshold;
	} m_Constants;

	void Initialize()
	{
		FSamplerDesc DefaultSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		FSamplerDesc PointSampler(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER);
		PointSampler.SetPointBorderDesc(Vector4f(0.f, 0.f, 0.f, 0.f));
		m_CSSignature.Reset(3, 2);
		m_CSSignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_ALL); // compute shader need 'all'
		m_CSSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
		m_CSSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
		m_CSSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_ALL);
		m_CSSignature.InitStaticSampler(1, PointSampler, D3D12_SHADER_VISIBILITY_ALL);
		m_CSSignature.Finalize(L"PostProcessing-Bloom");

		m_UpSampleCS = D3D12RHI::Get().CreateShader(L"../Resources/Shaders/PostProcess.hlsl", "CS_UpSample", "cs_5_1");
		m_CSUpSamplePSO.SetRootSignature(m_CSSignature);
		m_CSUpSamplePSO.SetComputeShader(CD3DX12_SHADER_BYTECODE(m_UpSampleCS.Get()));
		m_CSUpSamplePSO.Finalize();

		m_DownSampleCS = D3D12RHI::Get().CreateShader(L"../Resources/Shaders/PostProcess.hlsl", "CS_DownSample", "cs_5_1");
		m_CSDownSamplePSO.SetRootSignature(m_CSSignature);
		m_CSDownSamplePSO.SetComputeShader(CD3DX12_SHADER_BYTECODE(m_DownSampleCS.Get()));
		m_CSDownSamplePSO.Finalize();

		m_ExtractBloomCS = D3D12RHI::Get().CreateShader(L"../Resources/Shaders/PostProcess.hlsl", "CS_ExtractBloom", "cs_5_1");
		m_CSExtractBloomPSO.SetRootSignature(m_CSSignature);
		m_CSExtractBloomPSO.SetComputeShader(CD3DX12_SHADER_BYTECODE(m_ExtractBloomCS.Get()));
		m_CSExtractBloomPSO.Finalize();

		m_BuildHZBCS = D3D12RHI::Get().CreateShader(L"../Resources/Shaders/HZB.hlsl", "CS_BuildHZB", "cs_5_1");
		m_CSBuildHZBPSO.SetRootSignature(m_CSSignature);
		m_CSBuildHZBPSO.SetComputeShader(CD3DX12_SHADER_BYTECODE(m_BuildHZBCS.Get()));
		m_CSBuildHZBPSO.Finalize();

		uint32_t Width = WindowWin32::Get().GetWidth();
		uint32_t Height = WindowWin32::Get().GetHeight();
		for (int i = 0; i < _countof(g_BloomBuffers); ++i)
		{
			wchar_t Name[256];
			wsprintf(Name, L"Bloom_%d_1/%d", i, 1 << i);
			g_BloomBuffers[i].Create(Name, Width, Height, 1, DXGI_FORMAT_R11G11B10_FLOAT);

			Width >>= 1;
			Height >>= 1;
		}

		// post Process
		m_PostProcessSignature.Reset(2, 2);
		m_PostProcessSignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_PIXEL);
		m_PostProcessSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10);
		m_PostProcessSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		m_PostProcessSignature.InitStaticSampler(1, PointSampler, D3D12_SHADER_VISIBILITY_PIXEL);
		m_PostProcessSignature.Finalize(L"PostProcess");

		m_ScreenQuadVS = D3D12RHI::Get().CreateShader(L"../Resources/Shaders/PostProcess.hlsl", "VS_ScreenQuad", "vs_5_1");
		m_ToneMapWithBloomPS = D3D12RHI::Get().CreateShader(L"../Resources/Shaders/PostProcess.hlsl", "PS_ToneMapAndBloom", "ps_5_1");

		m_ToneMapWithBloomPSO.SetRootSignature(m_PostProcessSignature);
		m_ToneMapWithBloomPSO.SetRasterizerState(FPipelineState::RasterizerTwoSided);
		m_ToneMapWithBloomPSO.SetBlendState(FPipelineState::BlendDisable);
		m_ToneMapWithBloomPSO.SetDepthStencilState(FPipelineState::DepthStateDisabled);
		// no need to set input layout
		m_ToneMapWithBloomPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		m_ToneMapWithBloomPSO.SetRenderTargetFormats(1, &RenderWindow::Get().GetColorFormat(), DXGI_FORMAT_UNKNOWN);
		m_ToneMapWithBloomPSO.SetVertexShader(CD3DX12_SHADER_BYTECODE(m_ScreenQuadVS.Get()));
		m_ToneMapWithBloomPSO.SetPixelShader(CD3DX12_SHADER_BYTECODE(m_ToneMapWithBloomPS.Get()));
		m_ToneMapWithBloomPSO.Finalize();

		int Black = 0;
		m_BlackTexture.Create(1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &Black);

		int32_t ScreenWidth = WindowWin32::Get().GetWidth();
		int32_t ScreenHeight = WindowWin32::Get().GetHeight();
		int32_t NumMipsX = std::max((int32_t)std::ceil(std::log2(ScreenWidth) - 1.0), 1);
		int32_t NumMipsY = std::max((int32_t)std::ceil(std::log2(ScreenHeight) - 1.0), 1);
		int32_t NumMips = std::max(NumMipsX, NumMipsY);
		int32_t HZBWidth = 1 << NumMipsX;
		int32_t HZBHeight = 1 << NumMipsY;
		g_HiZBuffer.Create(L"HZB", HZBWidth, HZBHeight, NumMips, DXGI_FORMAT_R32G32_FLOAT);

		m_SSRPS = D3D12RHI::Get().CreateShader(L"../Resources/Shaders/SSR.hlsl", "PS_SSR", "ps_5_1");

		m_SSRPSO.SetRootSignature(m_PostProcessSignature);
		m_SSRPSO.SetRasterizerState(FPipelineState::RasterizerTwoSided);
		m_SSRPSO.SetBlendState(FPipelineState::BlendDisable);
		m_SSRPSO.SetDepthStencilState(FPipelineState::DepthStateDisabled);
		// no need to set input layout
		m_SSRPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		m_SSRPSO.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);
		m_SSRPSO.SetVertexShader(CD3DX12_SHADER_BYTECODE(m_ScreenQuadVS.Get()));
		m_SSRPSO.SetPixelShader(CD3DX12_SHADER_BYTECODE(m_SSRPS.Get()));
		m_SSRPSO.Finalize();
	}

	void Destroy()
	{

	}

	void PostProcessing::Render(FCommandContext& GfxContext)
	{
		if (g_EnableBloom)
		{
			GenerateBloom(GfxContext);
		}
		ToneMapping(GfxContext);
	}

	void GenerateBloom(FCommandContext& GfxContext)
	{
		GfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
		FComputeContext& ComputeContext = GfxContext.GetComputeContext();

		// extract bloom
		ComputeContext.SetRootSignature(m_CSSignature);
		ComputeContext.SetPipelineState(m_CSExtractBloomPSO);

		ComputeContext.TransitionResource(g_BloomBuffers[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

		m_Constants.BloomIntensity = g_BloomIntensity;
		m_Constants.BloomThreshold = g_BloomThreshold;
		ComputeContext.SetDynamicConstantBufferView(0, sizeof(m_Constants), &m_Constants);

		ComputeContext.SetDynamicDescriptor(1, 0, g_SceneColorBuffer.GetSRV());
		ComputeContext.SetDynamicDescriptor(2, 0, g_BloomBuffers[0].GetUAV());

		ComputeContext.Dispatch2D(g_BloomBuffers[0].GetWidth(), g_BloomBuffers[0].GetHeight());
		ComputeContext.TransitionResource(g_BloomBuffers[0], D3D12_RESOURCE_STATE_COMMON, true);

		//downsample
		ComputeContext.SetRootSignature(m_CSSignature);
		ComputeContext.SetPipelineState(m_CSDownSamplePSO);

		for (int i = 1; i < 5; ++i)
		{
			ComputeContext.TransitionResource(g_BloomBuffers[i - 1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			ComputeContext.TransitionResource(g_BloomBuffers[i], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			ComputeContext.SetDynamicDescriptor(1, 0, g_BloomBuffers[i-1].GetSRV());
			ComputeContext.SetDynamicDescriptor(2, 0, g_BloomBuffers[i].GetUAV());

			ComputeContext.Dispatch2D(g_BloomBuffers[i].GetWidth(), g_BloomBuffers[i].GetHeight());
		}

		//upsample
		ComputeContext.SetRootSignature(m_CSSignature);
		ComputeContext.SetPipelineState(m_CSUpSamplePSO);
		for (int i = 4; i > 0; --i)
		{
			ComputeContext.TransitionResource(g_BloomBuffers[i], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			ComputeContext.TransitionResource(g_BloomBuffers[i - 1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

			ComputeContext.SetDynamicDescriptor(1, 0, g_BloomBuffers[i].GetSRV());
			ComputeContext.SetDynamicDescriptor(2, 0, g_BloomBuffers[i - 1].GetUAV());

			ComputeContext.Dispatch2D(g_BloomBuffers[i - 1].GetWidth(), g_BloomBuffers[i - 1].GetHeight());

			ComputeContext.TransitionResource(g_BloomBuffers[i - 1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
		}
	}

	void ToneMapping(FCommandContext& GfxContext)
	{
		GfxContext.SetRootSignature(m_PostProcessSignature);
		GfxContext.SetPipelineState(m_ToneMapWithBloomPSO);
		GfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		GfxContext.SetViewportAndScissor(0, 0, WindowWin32::Get().GetWidth(), WindowWin32::Get().GetHeight());

		RenderWindow& renderWindow = RenderWindow::Get();
		FColorBuffer& BackBuffer = renderWindow.GetBackBuffer();

		GfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		if (g_EnableBloom)
		{
			GfxContext.TransitionResource(g_BloomBuffers[0], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
		GfxContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		GfxContext.SetRenderTargets(1, &BackBuffer.GetRTV());
		GfxContext.ClearColor(BackBuffer);

		m_Constants.BloomIntensity = g_BloomIntensity;
		m_Constants.BloomThreshold = g_BloomThreshold;
		GfxContext.SetDynamicConstantBufferView(0, sizeof(m_Constants), &m_Constants);

		GfxContext.SetDynamicDescriptor(1, 0, g_SceneColorBuffer.GetSRV());
		if (g_EnableBloom)
		{
			GfxContext.SetDynamicDescriptor(1, 1, g_BloomBuffers[0].GetSRV());
		}
		else
		{
			GfxContext.SetDynamicDescriptor(1, 1, m_BlackTexture.GetSRV());
		}

		GfxContext.Draw(3);
		GfxContext.TransitionResource(g_BloomBuffers[0], D3D12_RESOURCE_STATE_COMMON, true);
	}

	void BuildHZB(FCommandContext& CommandContext)
	{
		UserMarker GpuMarker(CommandContext, "BuildHZB");
		int32_t ScreenWidth = WindowWin32::Get().GetWidth();
		int32_t ScreenHeight = WindowWin32::Get().GetHeight();
		int32_t NumMipsX = std::max((int32_t)std::ceil(std::log2(ScreenWidth) - 1.0), 1);
		int32_t NumMipsY = std::max((int32_t)std::ceil(std::log2(ScreenHeight) - 1.0), 1);
		int32_t NumMips = std::max(NumMipsX, NumMipsY);
		int32_t HZBWidth = 1 << NumMipsX;
		int32_t HZBHeight = 1 << NumMipsY;

		CommandContext.TransitionResource(g_SceneDepthZ, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);

#if ASYNC_COMPUTE
		g_CommandListManager.GetComputeQueue().WaitForFenceValue(CommandContext.Flush());
		FComputeContext& ComputeContext = FComputeContext::Begin(L"Post Effects");
#else
		FComputeContext& ComputeContext = CommandContext.GetComputeContext();
#endif

		ComputeContext.SetRootSignature(m_CSSignature);
		ComputeContext.SetPipelineState(m_CSBuildHZBPSO);

		__declspec(align(16)) struct
		{
			Vector2f	SrcTexelSize;
			Vector2f	InputViewportMaxBound;
			float		Thickness;
		} Constants;

		for (int i = 0; i < NumMips; ++i)
		{
			if (i > 0)
				ComputeContext.TransitionSubResource(g_HiZBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, i - 1, true);
			ComputeContext.TransitionSubResource(g_HiZBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, i, true);
			ComputeContext.ClearUAV(g_HiZBuffer, i);

			ComputeContext.SetDynamicDescriptor(1, 0, i == 0 ? g_SceneDepthZ.GetDepthSRV() : g_HiZBuffer.GetMipSRV(i - 1));
			ComputeContext.SetDynamicDescriptor(2, 0, g_HiZBuffer.GetMipUAV(i));

			int32_t DstSizeX = DivideByMultiple(HZBWidth, uint32_t(1 << i));
			int32_t DstSizeY = DivideByMultiple(HZBHeight, uint32_t(1 << i));

			float SrcWidth = (float)g_SceneDepthZ.GetWidth();
			float SrcHeight = (float)g_SceneDepthZ.GetHeight();
			if (i > 0)
			{
				SrcWidth = (float)DivideByMultiple(HZBWidth, uint32_t(1 << (i - 1)));
				SrcHeight = (float)DivideByMultiple(HZBHeight, uint32_t(1 << (i - 1)));
			}
			Constants.SrcTexelSize = Vector2f(1.f / SrcWidth, 1.f / SrcHeight);

			Vector2f ViewportBound = Vector2f(1.f, 1.f);
			if (i == 0)
			{
				ViewportBound = Vector2f(1.f) - 0.5f * Constants.SrcTexelSize;
			}
			Constants.Thickness = g_Thickness;
			Constants.InputViewportMaxBound = ViewportBound;
			ComputeContext.SetDynamicConstantBufferView(0, sizeof(Constants), &Constants);

			ComputeContext.Dispatch2D(DstSizeX, DstSizeY);

			ComputeContext.TransitionSubResource(g_HiZBuffer, D3D12_RESOURCE_STATE_COMMON, i, true);
		}

		//CommandContext.TransitionResource(g_SceneDepthZ, D3D12_RESOURCE_STATE_COMMON, true);
		//ComputeContext.Flush(true);
#if ASYNC_COMPUTE
		ComputeContext.Finish(true);
#endif
	}

	void GenerateSSR(FCommandContext& GfxContext, FCamera& Camera, FCubeBuffer& CubeBuffer)
	{
		BuildHZB(GfxContext);

		UserMarker GpuMarker(GfxContext, "GenerateSSR");
		// Set necessary state.
		GfxContext.SetRootSignature(m_PostProcessSignature);
		GfxContext.SetPipelineState(m_SSRPSO);
		GfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// jitter offset
		GfxContext.SetViewportAndScissor(0, 0, g_SSRBuffer.GetWidth(), g_SSRBuffer.GetHeight());

		FColorBuffer& SceneBuffer = g_SceneColorBuffer;

		// Indicate that the back buffer will be used as a render target.
		GfxContext.TransitionResource(g_SSRBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		GfxContext.TransitionResource(g_GBufferA, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		GfxContext.TransitionResource(g_GBufferB, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		GfxContext.TransitionResource(g_GBufferC, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		GfxContext.TransitionResource(g_SceneDepthZ, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		GfxContext.TransitionResource(g_HiZBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		GfxContext.TransitionResource(CubeBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		GfxContext.TransitionResource(MotionBlur::g_VelocityBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		GfxContext.TransitionResource(TemporalEffects::GetHistoryBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

		GfxContext.SetRenderTargets(1, &g_SSRBuffer.GetRTV());

		__declspec(align(16)) struct
		{
			FMatrix		ViewProj;
			FMatrix		InvViewProj;
			FMatrix		ClipToPreClipNoAA;
			Vector4f	RootSizeMipCount;
			Vector4f	HZBUvFactorAndInvFactor;
			Vector3f	CameraPos;
			float		Thickness;
			float		WorldThickness;
			float		CompareTolerance;
			float		UseHiZ;
			float		UseMinMaxZ;
			int			NumRays;
			int			FrameIndexMod8;
		} PSConstants;

		float FactorX = 1.f * g_SSRBuffer.GetWidth() / g_HiZBuffer.GetWidth();
		float FactorY = 1.f * g_SSRBuffer.GetHeight() / g_HiZBuffer.GetHeight();
		PSConstants.HZBUvFactorAndInvFactor = Vector4f(FactorX, FactorY, 1.f / FactorX, 1.f / FactorY);
		PSConstants.CameraPos = Camera.GetPosition();
		PSConstants.RootSizeMipCount = Vector4f((float)g_HiZBuffer.GetWidth(), (float)g_HiZBuffer.GetHeight(), (float)g_HiZBuffer.GetNumMips(), 0.f);
		PSConstants.ViewProj = Camera.GetViewProjMatrix();
		PSConstants.InvViewProj = PSConstants.ViewProj.Inverse();
		PSConstants.ClipToPreClipNoAA = Camera.GetClipToPrevClipNoAA();
		PSConstants.Thickness = g_Thickness;
		PSConstants.WorldThickness = g_WorldThickness;
		PSConstants.CompareTolerance = g_CompareTolerance;
		PSConstants.UseHiZ = g_UseHiZ ? 1.f : 0.f;
		PSConstants.UseMinMaxZ = g_UseMinMaxZ ? 1.f : 0.f;
		PSConstants.NumRays = g_NumRays;
		PSConstants.FrameIndexMod8 = TemporalEffects::GetFrameIndex() % 8;

		GfxContext.SetDynamicConstantBufferView(0, sizeof(PSConstants), &PSConstants);
		GfxContext.SetDynamicDescriptor(1, 0, g_GBufferA.GetSRV());
		GfxContext.SetDynamicDescriptor(1, 1, g_GBufferB.GetSRV());
		GfxContext.SetDynamicDescriptor(1, 2, g_GBufferC.GetSRV());
		GfxContext.SetDynamicDescriptor(1, 3, g_SceneDepthZ.GetDepthSRV());
		GfxContext.SetDynamicDescriptor(1, 4, g_HiZBuffer.GetSRV());
		GfxContext.SetDynamicDescriptor(1, 5, TemporalEffects::GetHistoryBuffer().GetSRV());
		GfxContext.SetDynamicDescriptor(1, 6, MotionBlur::g_VelocityBuffer.GetSRV());
		GfxContext.SetDynamicDescriptor(1, 7, CubeBuffer.GetCubeSRV(0));

		GfxContext.Draw(3);
	}

}



