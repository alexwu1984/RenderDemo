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
	float		Exposure = 1.0;
	int			MipLevel = 0 ;
	int			MaxMipLevel = 1;
	int			NumSamplesPerDir = 0;
	int			Degree = 1;
	Vector3f	CameraPos;
	Vector4f	Coeffs[16];
} ;
extern EPSConstants g_EPSConstants;