#pragma once
#include "MathLib.h"

__declspec(align(16)) struct EVN_VS_CB
{
	FMatrix ModelMatrix;
	FMatrix ViewProjMatrix;
};
extern EVN_VS_CB EVN_VS;

__declspec(align(16)) struct EVN_PS_CB
{
	float Exposure;
	int MipLevel;
	int MaxMipLevel;
	int NumSamplesPerDir;
	int Degree;
	Vector3f	CameraPos;
	int		bSHDiffuse;
	Vector3f	pad;
	Vector4f	Coeffs[16];
};

extern EVN_PS_CB EVN_PS;