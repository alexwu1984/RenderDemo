#pragma once

#include "MathLib.h"

class FCamera
{
public:
	FCamera();
	FCamera(const Vector3f& CamPosition, const Vector3f& LookAtPosition, const Vector3f& UpDirection);

	void Update(float DeltaTime);

	Vector4f GetPosition() const;
	FMatrix GetViewMatrix() const;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	void Orbit(float Yaw, float Pitch);
	void Rotate(float Yaw, float Pitch);
	void Rotate(float radius, float theta, float phi);
	Vector3f GetUp() const { return m_Up; }
	Vector3f GetFocus() const { return m_Position + m_Forward * m_CameraLength; }

	void SetVerticalFov(float VerticalFov);
	void SetAspectRatio(float AspectHByW);
	void SetNearFar(float NearZ, float FarZ);
	void SetPerspectiveParams(float VerticalFov, float AspectHByW, float NearZ, float FarZ);
	const FMatrix GetProjectionMatrix() const;

	float GetNearClip() const { return m_NearZ; }
	float GetFarClip() const { return m_FarZ; }

	float GetFovY() const { return m_VerticalFov; }

	const FMatrix& GetViewProjMatrix() const { return m_ViewProjMatrix; }

	const FMatrix& GetPreviousViewMatrix() const { return m_PreviousViewMat; }
	const FMatrix& GetPreviousProjectionMatrix() const { return m_PreviousProjMat; }
	const FMatrix& GetPreviousViewProjMatrix() const { return m_PreviousViewProjMatrix; }
	const FMatrix& GetClipToPrevClip() const { return m_ClipToPrevClip; }
	const FMatrix& GetClipToPrevClipNoAA() const { return m_ClipToPrevClipNoAA; }

	void SetMouseMoveSpeed(float Speed) { m_MoveSpeed = Speed; }
	void SetMouseZoomSpeed(float Speed) { m_ZoomSpeed = Speed; }
	void SetMouseRotateSpeed(float Speed) { m_RotateSpeed = Speed; }
private:
	void UpdateViewMatrix();
	void UpdateProjMatrix();
	void ProcessMouseMove(float DeltaTime);
	void UpdateAllMatrix();

private:
	FMatrix m_ViewMat;
	FMatrix m_ProjMat;
	FMatrix m_ViewProjMatrix;

	float m_CameraLength;
	Vector3f m_Right, m_Up, m_Forward, m_Position;
	float m_Yaw = 90.0f;
	float m_Pitch = 0;

	float m_VerticalFov, m_AspectHByW;
	float m_NearZ, m_FarZ;

	float m_MoveSpeed = 5e-3f;
	float m_RotateSpeed = 0.001;
	float m_ZoomSpeed = 0.001;

	// The view-projection matrix from the previous frame
	FMatrix m_PreviousViewMat;
	FMatrix m_PreviousProjMat;
	FMatrix m_PreviousViewProjMatrix;

	// Projects a clip-space coordinate to the previous frame (useful for temporal effects).
	FMatrix m_ClipToPrevClip;
	FMatrix m_ClipToPrevClipNoAA;
};