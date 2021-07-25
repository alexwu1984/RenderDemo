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
#include "RenderPipelineInfo.h"
#include "Geometry.h"

#include <dxgi1_4.h>
#include <chrono>
#include <iostream>
#include <DirectXMath.h>


class Tutorial7 : public FGame
{
public:
	Tutorial7(const GameDesc& Desc) : FGame(Desc)
		, m_Camera(Vector3f(0.f, 0.f, -5.f), Vector3f(0.f, 0.0f, 0.f), Vector3f(0.f, 1.f, 0.f))
	{
	}

	virtual void OnStartup()
	{
		std::shared_ptr<FShader> shader = FShaderMgr::Get().CreateShader("phongFragment", L"../Resources/Shaders/SphericalHarmonic.hlsl");
		m_TexutreRenderState = std::make_shared<RenderPipelineInfo>(shader);
		m_TexutreRenderState->SetupRenderTargetFormat(1, &RenderWindow::Get().GetColorFormat(), RenderWindow::Get().GetDepthFormat());
		m_TexutreRenderState->SetRasterizerState(FGraphicsPipelineState::RasterizerFront);

		SetupRootSignature();

		m_Actor = std::make_shared<FModel>(L"../Resources/Models/Marry/Marry.obj");
		m_Actor->InitRootIndex(1, 3);
		m_Actor->SetPosition(Vector3f(0, -2, 0));
		m_Actor->SetLightIntensity(0.5);

		float radius = sqrtf(5 * 5 + 10 * 10);
		m_lightPos = SphericalToCartesian(1.0f, m_SunTheta, m_SunPhi);
		m_Actor->SetLightDir(Vector3f(0,0,0)- m_lightPos);

		std::vector<D3D12_INPUT_ELEMENT_DESC> MeshLayout;
		m_Actor->GetMeshLayout(MeshLayout);
		m_TexutreRenderState->SetupPipeline(m_RootSignature, MeshLayout);
		m_TexutreRenderState->PipelineFinalize();

		SetupSkyBox();

		std::array<std::wstring, 6> p1;
		std::wstring path = L"../Resources/Textures/Skybox/";

		for (int i = 0; i < 6; i++)
		{
			int num = i * 3;
			p1[i] = path + L"rendered_" + std::to_wstring(num) + L".jpg";
		}
		m_skyBoxTex1.LoadFromFileForCube(p1);

		std::array<std::wstring, 6> p2;

		for (int i = 0; i < 6; i++)
		{
			int num = i * 3 + 1;
			p2[i] = path + L"rendered_" + std::to_wstring(num) + L".jpg";
		}
		m_skyBoxTex2.LoadFromFileForCube(p2);

		std::array<std::wstring, 6> p3;
		for (int i = 0; i < 6; i++)
		{
			int num = i * 3 + 2;
			p3[i] = path + L"rendered_" + std::to_wstring(num) + L".jpg";
		}
		m_skyBoxTex3.LoadFromFileForCube(p3);

		m_BasePassConstBuf.CreateUpload(L"BasePassInfo", sizeof(m_BasePassInfo));
		m_BasePassCpuHandle = m_BasePassConstBuf.CreateConstantBufferView(0, sizeof(m_BasePassInfo));

		m_SkyBoxBasePassConstBuf.CreateUpload(L"SkyBoxBasePassInfo", sizeof(m_BasePassInfo));
		m_SkyBoxBasePassCputHandle = m_BasePassConstBuf.CreateConstantBufferView(0, sizeof(m_BasePassInfo));

		m_SHConstBuf.CreateUpload(L"SHInfo", sizeof(m_SHInfo));
		m_SHCpuHandle = m_SHConstBuf.CreateConstantBufferView(0, sizeof(m_SHInfo));

		GetCoefs();
	}

	virtual void OnUpdate()
	{

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

		DrawNormalItem(CommandContext);

		CommandContext.Finish(true);

		RenderWindow::Get().Present();
	}

	void SetupRootSignature()
	{
		FSamplerDesc DefaultSamplerDesc;

		m_RootSignature.Reset(7, 1);
		m_RootSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
		m_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, D3D12_SHADER_VISIBILITY_ALL);
		m_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 1, D3D12_SHADER_VISIBILITY_ALL);
		m_RootSignature[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature[4].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature[5].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature[6].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		m_RootSignature.Finalize(L"RootSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		m_GeometrySignature.Reset(2, 1);
		m_GeometrySignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_VERTEX);
		m_GeometrySignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
		m_GeometrySignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		m_GeometrySignature.Finalize(L"Geometry", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
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

	void DrawNormalItem(FCommandContext& CommandContext)
	{
		//m_elapsedTime = MATH_PI / 4.f;
		m_BasePassInfo.modelMatrix = m_Actor->GetModelMatrix();
		m_BasePassInfo.viewMatrix = m_Camera.GetViewMatrix();

		const float FovVertical = MATH_PI / 4.f;
		m_BasePassInfo.projectionMatrix = FMatrix::MatrixPerspectiveFovLH(FovVertical, (float)GetDesc().Width / GetDesc().Height, 0.1f, 100.f);
		memcpy(m_BasePassConstBuf.Map(), &m_BasePassInfo, sizeof(m_BasePassInfo));
		// Set necessary state.
		CommandContext.SetRootSignature(m_RootSignature);
		CommandContext.SetViewportAndScissor(0, 0, m_GameDesc.Width, m_GameDesc.Height);

		CommandContext.SetDynamicDescriptor(0, 0, m_BasePassCpuHandle);

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


		m_Actor->SetDrawParam([this](FCommandContext& CommandContext, std::shared_ptr<FMaterial>) {
			CommandContext.SetDynamicDescriptor(4, 0, m_skyBoxTex1.GetSRV());
			CommandContext.SetDynamicDescriptor(5, 0, m_skyBoxTex2.GetSRV());
			CommandContext.SetDynamicDescriptor(6, 0, m_skyBoxTex3.GetSRV());

			CommandContext.SetDynamicDescriptor(2, 0, m_SHCpuHandle);
			});

		CommandContext.SetPipelineState(m_TexutreRenderState->GetPipelineState());
		m_Actor->Draw(CommandContext);

		RenderSky(CommandContext);

		CommandContext.TransitionResource(BackBuffer, D3D12_RESOURCE_STATE_PRESENT);
	}

	void RenderSky(FCommandContext& CommandContext)
	{
		m_BasePassInfo.modelMatrix = m_skyBox.GetModelMatrix();
		m_BasePassInfo.viewMatrix = m_Camera.GetViewMatrix();

		const float FovVertical = MATH_PI / 4.f;
		m_BasePassInfo.projectionMatrix = FMatrix::MatrixPerspectiveFovLH(FovVertical, (float)GetDesc().Width / GetDesc().Height, 0.1f, 100.f);
		memcpy(m_SkyBoxBasePassConstBuf.Map(), &m_BasePassInfo, sizeof(m_BasePassInfo));

		CommandContext.SetRootSignature(m_GeometrySignature);
		CommandContext.SetDynamicDescriptor(0, 0, m_SkyBoxBasePassCputHandle);

		CommandContext.TransitionResource(m_skyBoxTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
		CommandContext.SetDynamicDescriptor(1, 0, m_skyBoxTex.GetSRV());

		CommandContext.SetPipelineState(m_SkyRenderState->GetPipelineState());
		m_skyBox.Draw(CommandContext);
	}

	void GetCoefs()
	{
		std::wstring path = L"../Resources/Textures/Skybox/coefficients.txt";
		std::ifstream ifs(path);
		if (!ifs)
		{
			return;
		}
		int i = 0;
		float r, g, b;
		while (ifs >> r >> g >> b)
		{
			if (i < 16)
			{
				m_SHInfo.Coef[i] = Vector4f(r, g, b, 0);
				i++;
			}
			else
			{
				break;
			}

		}
		memcpy(m_SHConstBuf.Map(), &m_SHInfo, sizeof(m_SHInfo));
	}

	virtual void OnShutdown()
	{

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

	POINT m_LastMousePos;
	FCamera m_Camera;

	float m_SunTheta = 1.25f * MATH_PI;
	float m_SunPhi = MATH_PI / 4;
	float m_DeltaTime = 0;

	float m_Theta = 1.5f * MATH_PI;
	float m_Phi = MATH_PI / 2 - 0.1f;
	float m_Radius = 50.0f;

	struct
	{
		FMatrix projectionMatrix;
		FMatrix modelMatrix;
		FMatrix viewMatrix;
		int mUseTex;
		Vector3i pad;
	} m_BasePassInfo;
	
	FConstBuffer m_BasePassConstBuf;
	D3D12_CPU_DESCRIPTOR_HANDLE m_BasePassCpuHandle;
	FConstBuffer m_SkyBoxBasePassConstBuf;
	D3D12_CPU_DESCRIPTOR_HANDLE m_SkyBoxBasePassCputHandle;

	struct
	{
		Vector4f Coef[16];
	}m_SHInfo;
	FConstBuffer m_SHConstBuf;
	D3D12_CPU_DESCRIPTOR_HANDLE m_SHCpuHandle;

	FRootSignature m_RootSignature;
	FRootSignature m_GeometrySignature;

	float m_elapsedTime = 0;
	std::chrono::high_resolution_clock::time_point tStart, tEnd;

	Vector3f m_lightPos;

	std::shared_ptr< RenderPipelineInfo> m_TexutreRenderState;
	std::shared_ptr< FModel > m_Actor;

	FTexture m_skyBoxTex;
	FGeometry m_skyBox;

	FTexture m_skyBoxTex1;
	FTexture m_skyBoxTex2;
	FTexture m_skyBoxTex3;

	std::shared_ptr< RenderPipelineInfo> m_SkyRenderState;
};

int main()
{
	ThrowIfFailed(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
	GameDesc Desc;
	Desc.Caption = L"Tutorial 7 - Mesh Loader";
	Tutorial7 tutorial(Desc);
	ApplicationWin32::Get().Run(&tutorial);
	CoUninitialize();
	return 0;
}