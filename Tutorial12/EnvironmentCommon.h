#pragma 
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
	int			MipLevel;
	int			MaxMipLevel;
	int			NumSamplesPerDir;
	int			Degree;
	Vector3f	CameraPos;
	Vector4f	Coeffs[16];
} ;
extern EPSConstants g_EPSConstants;