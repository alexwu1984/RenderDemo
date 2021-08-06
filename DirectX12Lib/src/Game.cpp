#include "Game.h"
#include "ApplicationWin32.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include <iostream>

FGame::FGame(const GameDesc& Desc)
	: m_GameDesc(Desc)
{
}

FGame::~FGame()
{
}

void FGame::OnStartup()
{
	std::cout << "OnStartup" << std::endl;
}

void FGame::OnUpdate()
{
}

void FGame::OnRender()
{
}

void FGame::OnShutdown()
{
	std::cout << "OnShutdown" << std::endl;
}

FDirectLightGameMode::FDirectLightGameMode(const GameDesc& Desc)
	:FGame(Desc), m_Camera(Vector3f(0.f, 0.f, -1.f), Vector3f(0.f, 0.0f, 0.f), Vector3f(0.f, 1.f, 0.f))
{
	m_Camera.SetMouseRotateSpeed(0.001);
	m_Camera.SetMouseZoomSpeed(0.001);
}


void FDirectLightGameMode::OnKeyDown(uint8_t Key)
{
	const float dt = m_DeltaTime * 20;

	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		m_LightInfo.SunTheta -= 1.0f * dt;

	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		m_LightInfo.SunTheta += 1.0f * dt;

	if (GetAsyncKeyState(VK_UP) & 0x8000)
		m_LightInfo.SunPhi -= 1.0f * dt;

	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		m_LightInfo.SunPhi += 1.0f * dt;

	m_LightInfo.SunPhi = Clamp(m_LightInfo.SunPhi, 0.1f, MATH_PI / 2);
}

void FDirectLightGameMode::OnRender()
{
	m_TEnd = std::chrono::high_resolution_clock::now();
	m_Delta = std::chrono::duration<float, std::milli>(m_TEnd - m_TStart).count();
	m_Camera.Update(m_Delta);
	if (m_Delta < (1000.0f / 400.0f))
	{
		return;
	}

	m_TStart = std::chrono::high_resolution_clock::now();

	// Update Uniforms
	m_DeltaTime = 0.001f * m_Delta;
	m_ElapsedTime += m_DeltaTime;
	m_ElapsedTime = fmodf(m_ElapsedTime, 6.283185307179586f);


	FCommandContext& CommandContext = FCommandContext::Begin();

	DoRender(CommandContext);

	CommandContext.TransitionResource(RenderWindow::Get().GetBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
	CommandContext.Finish(true);
	RenderWindow::Get().Present();

}

void FDirectLightGameMode::OnUpdate()
{
	Vector3f LightPos = SphericalToCartesian(1.0f, m_LightInfo.SunTheta, m_LightInfo.SunPhi);
	Vector3f LightUp(0.0f, 1.0f, 0.0f);
	Vector3f TargetPos(0, 0, 0);
	FCamera LightCamera(LightPos, TargetPos, LightUp);

	FMatrix LightView = LightCamera.GetViewMatrix();
	Vector3f SphereCenterLS = LightView.TransformPosition(TargetPos);

	// Ortho frustum in light space encloses scene.
	float l = SphereCenterLS.x - m_MainRadius;
	float b = SphereCenterLS.y - m_MainRadius;
	float n = SphereCenterLS.z - m_MainRadius;
	float r = SphereCenterLS.x + m_MainRadius;
	float t = SphereCenterLS.y + m_MainRadius;
	float f = SphereCenterLS.z + m_MainRadius;

	m_LightInfo.ProjectionMatrix = FMatrix::MatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	m_LightInfo.LightDir = TargetPos - LightPos;
	m_LightInfo.ViewMatrix = LightView;
	
}

