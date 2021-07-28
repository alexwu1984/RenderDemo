#pragma once

#include <d3dcompiler.h>
#include "MathLib.h"

class FCamera
{
public:
	FCamera() {}
	FCamera(const Vector3f& CamPosition, const Vector3f& LookAtPosition, const Vector3f& UpDirection);

	Vector4f GetPosition() const;
	FMatrix GetViewMatrix() const;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	void Orbit(float Yaw, float Pitch);
	void Rotate(float Yaw, float Pitch);
	void Rotate(float radius, float theta, float phi);
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
	Vector3f GetUp() const { return Up; }
	Vector3f GetFocus() const { return Position + Forward * CameraLength; }
private:
	void UpdateViewMatrix();

private:
	FMatrix ViewMat;
	float CameraLength;
	Vector3f Right, Up, Forward, Position;
	float MouseSensitivity = 0.05;
	float Yaw = 90.0f;
	float Pitch = 0;
};