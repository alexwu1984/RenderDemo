#include "Camera.h"
#include <stdio.h>
#include "glm/glm.hpp"
#include "GameInput.h"

FCamera::FCamera()
{
	SetPerspectiveParams(MATH_PI / 4.f, 1.f, 0.01f, 1000.f);
}

FCamera::FCamera(const Vector3f& CamPosition, const Vector3f& LookAtPosition, const Vector3f& UpDirection)
{
	m_Position = CamPosition;

	m_Up = UpDirection.Normalize();
	m_Forward = LookAtPosition - CamPosition;
	m_CameraLength = m_Forward.Length();
	m_Forward = m_Forward.Normalize();
	m_Right = Cross(m_Up, m_Forward);
	m_Up = Cross(m_Forward, m_Right);

	UpdateViewMatrix();
}


void FCamera::Update(float DeltaTime)
{
	float MoveDelta = float(m_MoveSpeed * DeltaTime);
	if (GameInput::IsKeyDown('W'))
	{
		this->MoveForward(MoveDelta);
	}
	if (GameInput::IsKeyDown('S'))
	{
		this->MoveForward(-MoveDelta);
	}
	if (GameInput::IsKeyDown('A'))
	{
		this->MoveRight(-MoveDelta);
	}
	if (GameInput::IsKeyDown('D'))
	{
		this->MoveRight(MoveDelta);
	}
	if (GameInput::IsKeyDown('Q'))
	{
		this->MoveUp(MoveDelta);
	}
	if (GameInput::IsKeyDown('E'))
	{
		this->MoveUp(-MoveDelta);
	}

	ProcessMouseMove(DeltaTime);
}

Vector4f FCamera::GetPosition() const
{
	return Vector4f(m_Position, 1.f);
}

void FCamera::UpdateViewMatrix()
{
	Vector3f Focus = m_Position + m_Forward * m_CameraLength;
	m_ViewMat = FMatrix::MatrixLookAtLH(m_Position, Focus, m_Up);
}

FMatrix FCamera::GetViewMatrix() const
{
	return m_ViewMat;
}

void FCamera::MoveForward(float Value)
{
	Vector3f Delta = m_Forward * Value;
	m_Position += Delta;
	UpdateViewMatrix();
}

void FCamera::MoveRight(float Value)
{
	Vector3f Delta = m_Right * Value;
	m_Position += Delta;
	UpdateViewMatrix();
}

void FCamera::MoveUp(float Value)
{
	Vector3f Delta = m_Up * Value;
	m_Position += Delta;
	UpdateViewMatrix();
}

void FCamera::Orbit(float Yaw, float Pitch)
{
	printf("Orbit %f %f\n", Yaw, Pitch);
	Vector3f Focus = m_Position + m_Forward * m_CameraLength;
	float D0 = -m_Right.Dot(Focus);
	float D1 = -m_Up.Dot(Focus);
	float D2 = -m_Forward.Dot(Focus);

	// 1. to orbit space
	FMatrix ToOrbit(Vector4f(m_Right, D0), Vector4f(m_Up, D1), Vector4f(m_Forward, D2), Vector4f(0.f, 0.f, 0.f, 1.f));
    ToOrbit = ToOrbit.Transpose();

	// 3. translate camera in orbit space
	FMatrix CameraTranslate = FMatrix::TranslateMatrix(Vector3f(0.f, 0.f, -m_CameraLength));
	FMatrix Result = CameraTranslate;

	// 2. rotate in orbit space
	FMatrix Rot = FMatrix::MatrixRotationRollPitchYaw(0, Pitch, Yaw);
	Result *= Rot;

	// 4. translate to world space
	FMatrix OrbitToWorld = FMatrix(m_Right, m_Up, m_Forward, Focus);
	Result *= OrbitToWorld;

	m_Right = Result.r0;
	m_Up = Result.r1;
	m_Forward = Result.r2;
	m_Position = Result.r3;
	UpdateViewMatrix();
}

void FCamera::Rotate(float Yaw, float Pitch)
{
	printf("Rotate %f %f\n", Yaw, Pitch);
	// pitch, yaw, roll
	FMatrix Rot = FMatrix::MatrixRotationRollPitchYaw(0.f, Pitch, Yaw);
	FMatrix Trans(m_Right, m_Up, m_Forward, Vector3f(0.f));
	FMatrix Result = Rot * Trans;

	m_Right = Result.r0;
	m_Up = Result.r1;
	m_Forward = Result.r2;
	UpdateViewMatrix();
}

void FCamera::Rotate(float radius,float theta, float phi)
{
	// Convert Spherical to Cartesian coordinates.
	m_Position = SphericalToCartesian(radius, theta, phi);

	// Build the view matrix.
	Vector3f target;
	Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);

	Vector3f Focus = m_Position + m_Forward * m_CameraLength;
	m_ViewMat = FMatrix::MatrixLookAtLH(m_Position, Focus, m_Up);
}

void FCamera::SetVerticalFov(float VerticalFov)
{
	m_VerticalFov = VerticalFov;
	UpdateProjMatrix();
}

void FCamera::SetAspectRatio(float AspectHByW)
{
	m_AspectHByW = AspectHByW;
	UpdateProjMatrix();
}

void FCamera::SetNearFar(float NearZ, float FarZ)
{
	m_NearZ = NearZ;
	m_FarZ = FarZ;
	UpdateProjMatrix();
}

void FCamera::SetPerspectiveParams(float VerticalFov, float AspectHByW, float NearZ, float FarZ)
{
	m_VerticalFov = VerticalFov;
	m_AspectHByW = AspectHByW;
	m_NearZ = NearZ;
	m_FarZ = FarZ;
	UpdateProjMatrix();
}

const FMatrix FCamera::GetProjectionMatrix() const
{
	return m_ProjMat;
}

void FCamera::UpdateProjMatrix()
{
	m_ProjMat = FMatrix::MatrixPerspectiveFovLH(m_VerticalFov, m_AspectHByW, m_NearZ, m_FarZ);
}

void FCamera::ProcessMouseMove(float DeltaTime)
{
	bool LeftMouseButtonDown = GameInput::IsKeyDown(VK_LBUTTON);
	bool RightMouseButtonDown = GameInput::IsKeyDown(VK_RBUTTON);
	bool AltKeyDown = GameInput::IsKeyDown(VK_MENU);
	Vector2i MouseDelta = GameInput::GetMoveDelta();
	float RotateDelta = float(m_RotateSpeed * DeltaTime);
	if (MouseDelta.x != 0 || MouseDelta.y != 0)
	{
		if (RightMouseButtonDown)
		{
			// rotate camera
			this->Rotate(MouseDelta.x * RotateDelta, MouseDelta.y * RotateDelta);
		}
		else if (LeftMouseButtonDown && AltKeyDown)
		{
			// orbit camera around focus
			this->Orbit(MouseDelta.x * RotateDelta, MouseDelta.y * RotateDelta);
		}
		else if (LeftMouseButtonDown)
		{
			if (abs(MouseDelta.x) >= abs(MouseDelta.y))
			{
				this->Rotate(MouseDelta.x * RotateDelta, 0.f);
			}
			else
			{
				this->MoveForward(float(-MouseDelta.y * m_MoveSpeed * DeltaTime));
			}
		}
		GameInput::MouseMoveProcessed();
	}

	float Zoom = GameInput::ConsumeMouseZoom();
	if (Zoom != 0.f)
	{
		this->MoveForward(Zoom * m_ZoomSpeed);
	}
}
