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
	float		Exposure = 1;
	int			MipLevel = 0;
	int			MaxMipLevel = 1;
	int			NumSamplesPerDir = 0;
	int			Degree = 1;
	Vector3f	CameraPos;
	int			bSHDiffuse;
	Vector3f	pad;
	Vector4f	Coeffs[16];
	Vector3f	LightDir;
	int			EnableLight;
} ;
extern EPSConstants g_EPSConstants;