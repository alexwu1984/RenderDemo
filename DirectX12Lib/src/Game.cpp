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

}


void FDirectLightGameMode::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	::SetCapture(WindowWin32::Get().GetWindowHandle());
}

void FDirectLightGameMode::OnMouseUp(WPARAM btnState, int x, int y)
{
	::ReleaseCapture();
}

void FDirectLightGameMode::OnMouseMove(WPARAM btnState, int x, int y)
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
	float time = std::chrono::duration<float, std::milli>(m_TEnd - m_TStart).count();
	if (time < (1000.0f / 400.0f))
	{
		return;
	}

	m_TStart = std::chrono::high_resolution_clock::now();

	// Update Uniforms
	m_DeltaTime = 0.001f * time;
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

