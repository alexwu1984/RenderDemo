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
#include "MeshData.h"
#include "ObjLoader.h"
#include "Model.h"
#include "Shader.h"
#include "RenderWindow.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <chrono>
#include <iostream>
#include <DirectXMath.h>
#include "ShadowMap.h"
#include "RenderPipelineInfo.h"
#include "Geometry.h"
#include "BlurFilter.h"
#include "Material.h"

constexpr int32_t SHADOW_BUFFER_SIZE = 1024;

extern FCommandListManager g_CommandListManager;

enum EShadowMode
{
	SM_NONE,
	SM_PCF,
	SM_VSM
};

enum EGenVSMMode
{
	E_GenWithComputeShader,
	E_GenWithPixelShader,
};

class Tutorial6 : public FGame
{
public:
	Tutorial6(const GameDesc& Desc) : FGame(Desc)
		,m_Camera(Vector3f(0.f, 0.f, -5.f), Vector3f(0.f, 0.0f, 0.f), Vector3f(0.f, 1.f, 0.f))
	{
	}

	void OnStartup()
	{


		if (m_GenVSMMode == E_GenWithComputeShader)
		{
			std::shared_ptr<FShader> drawShadows = FShaderMgr::Get().CreateShader("shadows", L"../Resources/Shaders/shadows.hlsl");
			m_ShadowMapRenderState = std::make_shared<RenderPipelineInfo>(drawShadows);
		}
		else
		{
			std::shared_ptr<FShader> drawShadows = FShaderMgr::Get().CreateShader("shadows","vs_main","ps_main_sqrtshadow", L"../Resources/Shaders/shadows.hlsl");
			m_ShadowMapRenderState = std::make_shared<RenderPipelineInfo>(drawShadows);
		}

		std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShader("phongFragment",L"../Resources/Shaders/phongFragment.hlsl");
		m_TexutreRenderState = std::make_shared<RenderPipelineInfo>(shader);
		m_TexutreRenderState->SetupRenderTargetFormat(1, &RenderWindow::Get().GetColorFormat(), RenderWindow::Get().GetDepthFormat());
		m_TexutreRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerFront);

		//compute shader
		m_VSMConvertShader = FShaderMgr::Get().CreateShader("vsmconvertCS","cs_main", L"../Resources/Shaders/vsmconvertCS.hlsl");
		std::shared_ptr<FShader> BlurHorizontalShader = FShaderMgr::Get().CreateShader("HorzBlurCS", "HorzBlurCS", L"../Resources/Shaders/blurcs.hlsl");
		std::shared_ptr<FShader> BlurVerticalShader = FShaderMgr::Get().CreateShader("VertBlurCS", "VertBlurCS", L"../Resources/Shaders/blurcs.hlsl");
		m_BlurFilter = std::make_shared<FBlurFilter>(BlurHorizontalShader, BlurVerticalShader, SHADOW_BUFFER_SIZE, SHADOW_BUFFER_SIZE);
		m_VSMBuffer.Create(L"VSM Buffer", SHADOW_BUFFER_SIZE, SHADOW_BUFFER_SIZE, 1, DXGI_FORMAT_R8G8B8A8_UNORM);

		SetupRootSignature();

		m_Actor = std::make_shared<FModel>(L"../Resources/Models/Marry/Marry.obj");
		m_Actor->SetPosition(Vector3f(-1, 0, 0));
		m_Actor->SetLightIntensity(0.5);

		m_lightPos = SphericalToCartesian(1.0f, m_SunTheta, m_SunPhi);
		m_Actor->SetLightDir(Vector3f(0,0,0)- m_lightPos);
		m_Actor->SetShadowType(m_ShadowMode);

		shader = FShaderMgr::Get().CreateShader("phongColorFragment",L"../Resources/Shaders/phongColorFragment.hlsl");
		m_ColorRenderState = std::make_shared<RenderPipelineInfo>(shader);
		m_ColorRenderState->SetupRenderTargetFormat(1, &RenderWindow::Get().GetColorFormat(), RenderWindow::Get().GetDepthFormat());
		m_ColorRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerFront);

		m_Floor = std::make_shared<FModel>(L"../Resources/Models/floor/floor.obj");
		m_Floor->SetPosition(Vector3f(0, 0, 0));
		m_Floor->SetLightDir(Vector3f(0, 0, 0) - m_lightPos);
		m_Floor->SetLightIntensity(0.5);
		m_Floor->SetShadowType(m_ShadowMode);

		if (m_GenVSMMode == E_GenWithComputeShader)
		{
			m_ShaderMap = std::make_shared<FShadowMap>(SHADOW_BUFFER_SIZE, SHADOW_BUFFER_SIZE);
			m_ShadowMapRenderState->SetupRenderTargetFormat(0, nullptr, DXGI_FORMAT_D24_UNORM_S8_UINT);
			m_ShadowMapRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerShadow);
			m_ShadowMapRenderState->SetBlendState(FPipelineState::BlendNoColorWrite);
		}

		std::shared_ptr<FShader> shadowDebug = FShaderMgr::Get().CreateShader("shadowdebug", L"../Resources/Shaders/ShadowDebug.hlsl");
		m_ShadowDebugRenderState = std::make_shared<RenderPipelineInfo>(shadowDebug);
		m_ShadowDebugRenderState->SetupRenderTargetFormat(1, &RenderWindow::Get().GetColorFormat(), RenderWindow::Get().GetDepthFormat());

		m_DebugBox = std::make_shared<FGeometry>();
		m_DebugBox->CreateRectange();
		m_DebugBox->SetPosition(Vector3f(2, 0, 0));


		{
			std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
			m_Actor->GetMeshLayout(MeshLayout);
			m_TexutreRenderState->SetupPipeline(m_RootSignature, MeshLayout);
			m_ShadowMapRenderState->SetupPipeline(m_RootSignature, MeshLayout);

			m_TexutreRenderState->PipelineFinalize();
			m_ShadowMapRenderState->PipelineFinalize();
		}

		{
			std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
			m_Floor->GetMeshLayout(MeshLayout);
			m_ColorRenderState->SetupPipeline(m_RootSignature, MeshLayout);
			m_ColorRenderState->PipelineFinalize();
		}

		{
			std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
			m_DebugBox->GetMeshLayout(MeshLayout);
			m_ShadowDebugRenderState->SetupPipeline(m_GeometrySignature, MeshLayout);
			m_ShadowDebugRenderState->PipelineFinalize();
		}

		SetupSkyBox();

	}

	void OnShutdown()
	{
	}

	void OnUpdate()
	{

	}

	void SetupSkyBox()
	{
		std::array<std::wstring, 6> faces
		{
			L"../Resources/Textures/Skybox/right.jpg",
			L"../Resources/Textures/Skybox/left.jpg",
			L"../Resources/Textures/Skybox/top.jpg",
			L"../Resources/Textures/Skybox/bottom.jpg",
			L"../Resources/Textures/Skybox/front.jpg",
			L"../Resources/Textures/Skybox/back.jpg"
		};

		m_skyBoxTex.LoadFromFileForCube(faces);
		
		std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShader("sky", L"../Resources/Shaders/sky.hlsl");
		m_SkyRenderState = std::make_shared<RenderPipelineInfo>(shader);
		m_SkyRenderState->SetupRenderTargetFormat(1, &RenderWindow::Get().GetColorFormat(), RenderWindow::Get().GetDepthFormat());
		m_SkyRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerTwoSided);
		m_SkyRenderState->SetDepthStencilState(FGraphicsPipelineState::DepthStateTwoSided);

		std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
		m_skyBox.CreateCube();
		m_skyBox.GetMeshLayout(MeshLayout);

		m_SkyRenderState->SetupPipeline(m_GeometrySignature, MeshLayout);
		m_SkyRenderState->PipelineFinalize();
	}
	
	void OnRender()
	{
		
		// Frame limit set to 60 fps
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
		
		DrawSceneToShadowMap(CommandContext);
		if (m_ShadowMode == SM_VSM)
		{
			ComputePass(CommandContext);
		}

		DrawNormalItem(CommandContext);
		
		CommandContext.Finish(true);

		RenderWindow::Get().Present();
	}

private:
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
		m_RootSignature[0].InitAsConstants(0, sizeof(m_uboVS) / 4, D3D12_SHADER_VISIBILITY_ALL);
		m_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_ALL);
		m_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature.InitStaticSampler(1, ShadowSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature.InitStaticSampler(2, ShadowSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature.Finalize(L"RootSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		m_GeometrySignature.Reset(2, 1);
		m_GeometrySignature[0].InitAsConstants(0, sizeof(m_uboVS) / 4, D3D12_SHADER_VISIBILITY_VERTEX);
		m_GeometrySignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
		m_GeometrySignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		m_GeometrySignature.Finalize(L"Geometry", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		m_CSSignature.Reset(2, 0);
		m_CSSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
		m_CSSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
		m_CSSignature.Finalize(L"Compute Shader RootSignature");
		
		m_VSMConvertPSO.SetRootSignature(m_CSSignature);
		m_VSMConvertPSO.SetComputeShader(CD3DX12_SHADER_BYTECODE(m_VSMConvertShader->GetComputeShader().Get()));
		m_VSMConvertPSO.Finalize();
		
	}


	void DrawNormalItem(FCommandContext& CommandContext)
	{
		if (m_ShadowMode == SM_VSM)
		{
			g_CommandListManager.GetGraphicsQueue().StallForProducer(g_CommandListManager.GetComputeQueue());
		}

		//m_elapsedTime = MATH_PI / 4.f;
		m_uboVS.modelMatrix = FMatrix::TranslateMatrix(Vector3f(0, -2, 0));
		m_uboVS.viewMatrix = m_Camera.GetViewMatrix();

		const float FovVertical = MATH_PI / 4.f;
		m_uboVS.projectionMatrix = FMatrix::MatrixPerspectiveFovLH(FovVertical, (float)GetDesc().Width / GetDesc().Height, 0.1f, 100.f);

		// Set necessary state.
		CommandContext.SetRootSignature(m_RootSignature);
		CommandContext.SetViewportAndScissor(0, 0, m_GameDesc.Width, m_GameDesc.Height);

		RenderWindow& renderWindow = RenderWindow::Get();
		FColorBuffer& BackBuffer = renderWindow.GetBackBuffer();
		FDepthBuffer& DepthBuffer = renderWindow.GetDepthBuffer();
		// Indicate that the back buffer will be used as a render target.
		CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		CommandContext.TransitionResource(DepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE,true);
		CommandContext.SetRenderTargets(1, &BackBuffer.GetRTV(), DepthBuffer.GetDSV());

		// Record commands.
		CommandContext.ClearColor(BackBuffer);
		CommandContext.ClearDepth(DepthBuffer);
		CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		if (m_Actor)
		{
			m_Actor->SetDrawParam([this](FCommandContext& CommandContext, std::shared_ptr<FMaterial> Material)
			{
					m_uboVS.mUseTex = Material->GetDiffuseTexture().GetResource() ? true : false;
					CommandContext.SetConstantArray(0, sizeof(m_uboVS) / 4, &m_uboVS);
					if (m_ShadowMode == SM_VSM)
					{
						CommandContext.TransitionResource(m_BlurFilter->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
						CommandContext.SetDynamicDescriptor(3, 0, m_BlurFilter->GetSRV());
					}
					else
					{
						CommandContext.SetDynamicDescriptor(3, 0, m_ShaderMap->Srv());
					}

			});
			CommandContext.SetPipelineState(m_TexutreRenderState->GetPipelineState());
			m_Actor->Draw(CommandContext);
		}

		if (m_Floor)
		{
			m_Floor->SetDrawParam([this](FCommandContext& CommandContext, std::shared_ptr<FMaterial> Material)
			{
					m_uboVS.mUseTex = Material->GetDiffuseTexture().GetResource() ? true : false;
					CommandContext.SetConstantArray(0, sizeof(m_uboVS) / 4, &m_uboVS);
					if (m_ShadowMode == SM_VSM)
					{
						CommandContext.TransitionResource(m_BlurFilter->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
						CommandContext.SetDynamicDescriptor(3, 0, m_BlurFilter->GetSRV());
					}
					else
					{
						CommandContext.SetDynamicDescriptor(3, 0, m_ShaderMap->Srv());
					}
			});
			CommandContext.SetPipelineState(m_ColorRenderState->GetPipelineState());
			m_Floor->Draw(CommandContext);
		}

		DrawShadowDebug(CommandContext);
		RenderSky(CommandContext);

		CommandContext.TransitionResource(m_BlurFilter->GetBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_PRESENT);
	}

	void DrawSceneToShadowMap(FCommandContext& CommandContext)
	{
		if (!m_Actor || !m_Floor)
		{
			return;
		}
		CommandContext.SetRootSignature(m_RootSignature);

		if (m_GenVSMMode == E_GenWithComputeShader)
		{
			CommandContext.SetViewportAndScissor(m_ShaderMap->Viewport(), m_ShaderMap->ScissorRect());
			CommandContext.TransitionResource(m_ShaderMap->GetBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE, true);

			CommandContext.ClearDepth(m_ShaderMap->GetBuffer());
			CommandContext.SetDepthStencilTarget(m_ShaderMap->Dsv());
		}


		CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		float radius = m_Actor->GetBoundBox().Extents.Length();
		Vector3f lightUp(0.0f, 1.0f, 0.0f);
		Vector3f targetPos(0,0,0);
		
		Vector3f LightDir = targetPos - m_lightPos;
		FCamera lightCamera(m_lightPos, targetPos, lightUp);


		FMatrix lightView = lightCamera.GetViewMatrix();
		Vector3f sphereCenterLS = lightView.TransformPosition(targetPos);

		// Ortho frustum in light space encloses scene.
		float l = sphereCenterLS.x - radius;
		float b = sphereCenterLS.y - radius;
		float n = sphereCenterLS.z - radius;
		float r = sphereCenterLS.x + radius;
		float t = sphereCenterLS.y + radius;
		float f = sphereCenterLS.z + radius;

		m_uboVS.modelMatrix = FMatrix::TranslateMatrix(Vector3f(0, -2, 0));
		m_uboVS.viewMatrix = lightView;
		m_uboVS.projectionMatrix = FMatrix::MatrixOrthographicOffCenterLH(l,r,b,t,n,f);

		CommandContext.SetPipelineState(m_ShadowMapRenderState->GetPipelineState());
		CommandContext.SetConstantArray(0, sizeof(m_uboVS) / 4, &m_uboVS);

		m_Actor->SetDrawParam([this](FCommandContext& , std::shared_ptr<FMaterial>){});
		m_Actor->SetLightMVP(m_uboVS.modelMatrix, m_uboVS.viewMatrix, m_uboVS.projectionMatrix);
		m_Actor->Draw(CommandContext);

		m_Floor->SetDrawParam([this](FCommandContext& , std::shared_ptr<FMaterial>){});
		m_Floor->SetLightMVP(m_uboVS.modelMatrix, m_uboVS.viewMatrix, m_uboVS.projectionMatrix);
		m_Floor->Draw(CommandContext);

		if (m_GenVSMMode == E_GenWithComputeShader)
		{
			CommandContext.TransitionResource(m_ShaderMap->GetBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ, true);
		}
	}

	void DrawShadowDebug(FCommandContext& CommandContext)
	{
		m_uboVS.modelMatrix = m_DebugBox->GetModelMatrix();
		m_uboVS.viewMatrix = m_Camera.GetViewMatrix();

		const float FovVertical = MATH_PI / 4.f;
		m_uboVS.projectionMatrix = FMatrix::MatrixPerspectiveFovLH(FovVertical, (float)GetDesc().Width / GetDesc().Height, 0.1f, 100.f);

		// Set necessary state.
		CommandContext.SetRootSignature(m_GeometrySignature);
		CommandContext.SetConstantArray(0, sizeof(m_uboVS) / 4, &m_uboVS);

		if (m_ShadowMode == SM_VSM)
		{
			CommandContext.TransitionResource(m_BlurFilter->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			CommandContext.SetDynamicDescriptor(1, 0, m_BlurFilter->GetSRV());
		}
		else
		{
			if (m_GenVSMMode == E_GenWithComputeShader)
			{
				CommandContext.SetDynamicDescriptor(1, 0, m_ShaderMap->Srv());
			}
		}

		// Record commands.
		CommandContext.SetPipelineState(m_ShadowDebugRenderState->GetPipelineState());
		m_DebugBox->Draw(CommandContext);

		if (m_ShadowMode == SM_VSM)
		{
			CommandContext.TransitionResource(m_BlurFilter->GetBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}

	}

	void RenderSky(FCommandContext& CommandContext)
	{
		m_uboVS.modelMatrix = m_skyBox.GetModelMatrix();
		m_uboVS.viewMatrix = m_Camera.GetViewMatrix();

		const float FovVertical = MATH_PI / 4.f;
		m_uboVS.projectionMatrix = FMatrix::MatrixPerspectiveFovLH(FovVertical, (float)GetDesc().Width / GetDesc().Height, 0.1f, 100.f);

		CommandContext.SetRootSignature(m_GeometrySignature);
		CommandContext.SetConstantArray(0, sizeof(m_uboVS) / 4, &m_uboVS);

		CommandContext.TransitionResource(m_skyBoxTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,true);
		CommandContext.SetDynamicDescriptor(1, 0, m_skyBoxTex.GetSRV());

		CommandContext.SetPipelineState(m_SkyRenderState->GetPipelineState());
		m_skyBox.Draw(CommandContext);
	}

	void ComputePass(FCommandContext& GfxContext)
	{
		if (m_GenVSMMode == E_GenWithComputeShader)
		{
			GfxContext.TransitionResource(m_ShaderMap->GetBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			g_CommandListManager.GetComputeQueue().WaitForFenceValue(GfxContext.Flush());

			FComputeContext& CommandContext = FComputeContext::Begin(L"Compute Queue");

			CommandContext.SetRootSignature(m_CSSignature);
			CommandContext.SetPipelineState(m_VSMConvertPSO);

			CommandContext.TransitionResource(m_VSMBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			CommandContext.SetDynamicDescriptor(0, 0, m_ShaderMap->Srv());
			CommandContext.SetDynamicDescriptor(1, 0, m_VSMBuffer.GetUAV());

			uint32_t GroupCount = (SHADOW_BUFFER_SIZE + 15) / 16;
			CommandContext.Dispatch(GroupCount, GroupCount, 1);

			m_BlurFilter->Execute(m_VSMBuffer, CommandContext, 2);
			CommandContext.Finish(false);

			GfxContext.TransitionResource(m_ShaderMap->GetBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
		}
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
		const float dt = m_DeltaTime*20;

		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
			m_SunTheta -= 1.0f * dt;

		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
			m_SunTheta += 1.0f * dt;

		if (GetAsyncKeyState(VK_UP) & 0x8000)
			m_SunPhi -= 1.0f * dt;

		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
			m_SunPhi += 1.0f * dt;

		m_SunPhi = Clamp(m_SunPhi, 0.1f, MATH_PI/2);
		m_lightPos = SphericalToCartesian(1.0f, m_SunTheta, m_SunPhi);
		Vector3f targetPos(0, 0, 0);
		Vector3f LightDir = targetPos - m_lightPos;
		if (m_Actor)
		{
			m_Actor->SetLightDir(LightDir);
		}
		if (m_Floor)
		{
			m_Floor->SetLightDir(LightDir);
		}
		

		if (GetAsyncKeyState('W') & 0x8000)
			m_Camera.MoveForward(1.0f * dt);

		if (GetAsyncKeyState('S') & 0x8000)
			m_Camera.MoveForward(-1.0f * dt);

		if (GetAsyncKeyState('A') & 0x8000)
			m_Camera.MoveRight(-1.0f * dt);

		if (GetAsyncKeyState('D') & 0x8000)
			m_Camera.MoveRight(1.0f * dt);
		
	}

private:
	struct
	{
		FMatrix projectionMatrix;
		FMatrix modelMatrix;
		FMatrix viewMatrix;
		int mUseTex;
		Vector3i pad;
	} m_uboVS;

	FRootSignature m_RootSignature;
	FRootSignature m_GeometrySignature;

	float m_elapsedTime = 0;
	std::chrono::high_resolution_clock::time_point tStart, tEnd;

	std::shared_ptr< FModel > m_Actor;
	std::shared_ptr< FModel > m_Floor;
	std::shared_ptr< FGeometry> m_DebugBox;
	std::shared_ptr< FShadowMap> m_ShaderMap;

	std::shared_ptr< RenderPipelineInfo> m_TexutreRenderState;
	std::shared_ptr< RenderPipelineInfo> m_ColorRenderState;
	std::shared_ptr< RenderPipelineInfo> m_ShadowMapRenderState;
	std::shared_ptr< RenderPipelineInfo> m_ShadowDebugRenderState;

	//computeshader
	FComputePipelineState m_VSMConvertPSO;
	std::shared_ptr<FShader> m_VSMConvertShader;
	std::shared_ptr<FBlurFilter> m_BlurFilter;

	FRootSignature m_CSSignature;
	FColorBuffer m_VSMBuffer;

	POINT m_LastMousePos;
	FCamera m_Camera;

	float m_SunTheta = 1.25f * MATH_PI;
	float m_SunPhi = MATH_PI / 4;
	float m_DeltaTime = 0;

	float m_Theta = 1.5f * MATH_PI;
	float m_Phi = MATH_PI/2 - 0.1f;
	float m_Radius = 50.0f;
	Vector3f m_lightPos;

	EShadowMode m_ShadowMode = SM_VSM;
	EGenVSMMode m_GenVSMMode = E_GenWithComputeShader;

	FTexture m_skyBoxTex;
	FGeometry m_skyBox;
	std::shared_ptr< RenderPipelineInfo> m_SkyRenderState;
};

int main()
{
	ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
	GameDesc Desc;
	Desc.Caption = L"Tutorial 6 - Mesh Loader";
	Tutorial6 tutorial(Desc);
	ApplicationWin32::Get().Run(&tutorial);
	CoUninitialize();
	return 0;
}