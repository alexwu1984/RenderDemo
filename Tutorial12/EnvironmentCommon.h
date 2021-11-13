#pragma once
#include "MathLib.h"

__declspec(align(16)) struct EVSConstants
{
	FMatrix ModelMatrix;
	FMatrix ViewProjMatrix;
	FMatrix PreviousModelMatrix;
	FMatrix PreviousViewProjMatrix;
	Vector2f ViewportSize;
} ;
extern EVSConstants g_EVSConstants;

__declspec(align(16)) struct EPSConstants
{
	float		Exposure = 1.0;
	Vector3f	CameraPos;
	Vector3f	BaseColor;
	float		Metallic = 1.0;
	float		Roughness = 0.0;
	int			MaxMipLevel;
	int			bSHDiffuse = false;
	int			Degree = 0;
	FMatrix		InvViewProj;
	Vector4f	TemporalAAJitter;
	Vector4f	Coeffs[16];
	Vector3f	LightDir;
	int			EnableLight;
	int         EnableSSR;
} ;
extern EPSConstants g_PBRPSConstants;