#pragma once
#include "MathLib.h"

__declspec(align(16)) struct EVSConstants
{
	FMatrix ModelMatrix;
	FMatrix ViewProjMatrix;
} ;
extern EVSConstants g_EVSConstants;

__declspec(align(16)) struct EPSConstants
{
	float		Exposure;
	Vector3f	CameraPos;
	Vector3f	BaseColor;
	float		Metallic;
	float		Roughness;
	int			MaxMipLevel;
	int			bSHDiffuse;
	int			Degree;
	FMatrix		InvViewProj;
	Vector4f	TemporalAAJitter;
	Vector4f	Coeffs[16];
	Vector3f	LightDir;
	int			EnableLight;

} ;
extern EPSConstants g_PBRPSConstants;