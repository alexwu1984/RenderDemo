#pragma once

#include "MathLib.h"
#include "Camera.h"

struct GameDesc
{
	std::wstring Caption = L"The Practice of Direct3D 12 Programming";
	int Width = 1024;
	int Height = 768;
};

class FCommandContext;


class FGame
{
public:
	FGame(const GameDesc& Desc);
	virtual ~FGame();

	const GameDesc& GetDesc() const { return m_GameDesc; }

	virtual void OnStartup();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnShutdown();

	virtual void OnKeyDown(uint8_t Key) {}
	virtual void OnKeyUp(uint8_t Key) {}
	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }

	int GetWidth() const { return m_GameDesc.Width; }
	int GetHeight() const { return m_GameDesc.Height; }
	std::wstring GetWindowTitle() const { return m_GameDesc.Caption; }

protected:
	GameDesc m_GameDesc;
};

class FDirectLightGameMode : public FGame
{
public:
	FDirectLightGameMode(const GameDesc& Desc);
	virtual ~FDirectLightGameMode() {};

public:
	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	virtual void OnKeyDown(uint8_t Key);
	virtual void DoRender(FCommandContext& CommandContext) {};
	virtual void OnRender();
	virtual void OnUpdate();

protected:
	struct LightInfo
	{
		Vector3f LightDir;
		FMatrix ProjectionMatrix;
		FMatrix ViewMatrix;
		float SunTheta = 1.25f * MATH_PI;
		float SunPhi = MATH_PI / 4;
	};

	LightInfo m_LightInfo;
	FCamera m_Camera;
	POINT m_LastMousePos;
	float m_DeltaTime = 0;
	float m_Theta = 1.5f * MATH_PI;
	float m_Phi = MATH_PI / 2 - 0.1f;
	float m_Radius = 50.0f;
	float m_ElapsedTime = 0;
	float m_MainRadius = 10;
	bool m_firstMouse = true;
	std::chrono::high_resolution_clock::time_point m_TStart, m_TEnd;
};

