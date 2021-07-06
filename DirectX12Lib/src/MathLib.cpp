#include "MathLib.h"

FMatrix::FMatrix()
	: r0(1.f, 0.f, 0.f, 0.f)
	, r1(0.f, 1.f, 0.f, 0.f)
	, r2(0.f, 0.f, 1.f, 0.f)
	, r3(0.f, 0.f, 0.f, 1.f)
{

}

FMatrix::FMatrix(const Vector3f& r0, const Vector3f& r1, const Vector3f& r2, const Vector3f& r3)
	: r0(Vector4f(r0, 0.f))
	, r1(Vector4f(r1, 0.f))
	, r2(Vector4f(r2, 0.f))
	, r3(Vector4f(r3, 1.f))
{

}

FMatrix::FMatrix(const Vector4f& r0, const Vector4f& r1, const Vector4f& r2, const Vector4f& r3)
	: r0(r0), r1(r1), r2(r2), r3(r3)
{

}

FMatrix::FMatrix(float m00, float m01, float m02, float m03, 
	float m10, float m11, float m12, float m13, 
	float m20, float m21, float m22, float m23, 
	float m30, float m31, float m32, float m33)
	: r0(Vector4f(m00, m01, m02, m03))
	, r1(Vector4f(m10, m11, m12, m13))
	, r2(Vector4f(m20, m21, m22, m23))
	, r3(Vector4f(m30, m31, m32, m33))
{
}

Vector4f FMatrix::Column(int i) const
{
	return Vector4f(r0[i], r1[i], r2[i], r3[i]);
}

FMatrix FMatrix::operator*(const FMatrix& rhs) const
{
	Vector4f c0 = rhs.Column(0);
	Vector4f c1 = rhs.Column(1);
	Vector4f c2 = rhs.Column(2);
	Vector4f c3 = rhs.Column(3);

	return FMatrix(
		r0.Dot(c0), r0.Dot(c1), r0.Dot(c2), r0.Dot(c3),
		r1.Dot(c0), r1.Dot(c1), r1.Dot(c2), r1.Dot(c3),
		r2.Dot(c0), r2.Dot(c1), r2.Dot(c2), r2.Dot(c3),
		r3.Dot(c0), r3.Dot(c1), r3.Dot(c2), r3.Dot(c3)
	);
}

FMatrix& FMatrix::operator*=(const FMatrix& rhs)
{
	*this = (*this) * rhs;
	return *this;
}

Vector3f FMatrix::TranslateVector(const Vector3f& vector)
{
	Vector4f Res = Vector4f(vector, 0.f) * (*this);
	return Vector3f(Res.x, Res.y, Res.z);
}

Vector3f FMatrix::TransformPosition(const Vector3f& position)
{
	Vector4f Res = Vector4f(position, 1.f) * (*this);
	return Vector3f(Res.x, Res.y, Res.z);
}

FMatrix FMatrix::Transpose()
{
	return FMatrix(
		r0.x, r1.x, r2.x, r3.x,
		r0.y, r1.y, r2.y, r3.y,
		r0.z, r1.z, r2.z, r3.z,
		r0.w, r1.w, r2.w, r3.w
	);
}

FMatrix FMatrix::Inverse()
{
	FMatrix mTrans;
	FMatrix tmp;
	float    fTemp[12], fDet;

	mTrans = Transpose();
	fTemp[0] = mTrans._22 * mTrans._33;
	fTemp[1] = mTrans._23 * mTrans._32;
	fTemp[2] = mTrans._21 * mTrans._33;
	fTemp[3] = mTrans._23 * mTrans._31;
	fTemp[4] = mTrans._21 * mTrans._32;
	fTemp[5] = mTrans._22 * mTrans._31;
	fTemp[6] = mTrans._20 * mTrans._33;
	fTemp[7] = mTrans._23 * mTrans._30;
	fTemp[8] = mTrans._20 * mTrans._32;
	fTemp[9] = mTrans._22 * mTrans._30;
	fTemp[10] = mTrans._20 * mTrans._31;
	fTemp[11] = mTrans._21 * mTrans._30;


	tmp._00 = fTemp[0] * mTrans._11 + fTemp[3] * mTrans._12 + fTemp[4] * mTrans._13;
	tmp._00 -= fTemp[1] * mTrans._11 + fTemp[2] * mTrans._12 + fTemp[5] * mTrans._13;
	tmp._01 = fTemp[1] * mTrans._10 + fTemp[6] * mTrans._12 + fTemp[9] * mTrans._13;
	tmp._01 -= fTemp[0] * mTrans._10 + fTemp[7] * mTrans._12 + fTemp[8] * mTrans._13;
	tmp._02 = fTemp[2] * mTrans._10 + fTemp[7] * mTrans._11 + fTemp[10] * mTrans._13;
	tmp._02 -= fTemp[3] * mTrans._10 + fTemp[6] * mTrans._11 + fTemp[11] * mTrans._13;
	tmp._03 = fTemp[5] * mTrans._10 + fTemp[8] * mTrans._11 + fTemp[11] * mTrans._12;
	tmp._03 -= fTemp[4] * mTrans._10 + fTemp[9] * mTrans._11 + fTemp[10] * mTrans._12;
	tmp._10 = fTemp[1] * mTrans._01 + fTemp[2] * mTrans._02 + fTemp[5] * mTrans._03;
	tmp._10 -= fTemp[0] * mTrans._01 + fTemp[3] * mTrans._02 + fTemp[4] * mTrans._03;
	tmp._11 = fTemp[0] * mTrans._00 + fTemp[7] * mTrans._02 + fTemp[8] * mTrans._03;
	tmp._11 -= fTemp[1] * mTrans._00 + fTemp[6] * mTrans._02 + fTemp[9] * mTrans._03;
	tmp._12 = fTemp[3] * mTrans._00 + fTemp[6] * mTrans._01 + fTemp[11] * mTrans._03;
	tmp._12 -= fTemp[2] * mTrans._00 + fTemp[7] * mTrans._01 + fTemp[10] * mTrans._03;
	tmp._13 = fTemp[4] * mTrans._00 + fTemp[9] * mTrans._01 + fTemp[10] * mTrans._02;
	tmp._13 -= fTemp[5] * mTrans._00 + fTemp[8] * mTrans._01 + fTemp[11] * mTrans._02;


	fTemp[0] = mTrans._02 * mTrans._13;
	fTemp[1] = mTrans._03 * mTrans._12;
	fTemp[2] = mTrans._01 * mTrans._13;
	fTemp[3] = mTrans._03 * mTrans._11;
	fTemp[4] = mTrans._01 * mTrans._12;
	fTemp[5] = mTrans._02 * mTrans._11;
	fTemp[6] = mTrans._00 * mTrans._13;
	fTemp[7] = mTrans._03 * mTrans._10;
	fTemp[8] = mTrans._00 * mTrans._12;
	fTemp[9] = mTrans._02 * mTrans._10;
	fTemp[10] = mTrans._00 * mTrans._11;
	fTemp[11] = mTrans._01 * mTrans._10;


	tmp._20 = fTemp[0] * mTrans._31 + fTemp[3] * mTrans._32 + fTemp[4] * mTrans._33;
	tmp._20 -= fTemp[1] * mTrans._31 + fTemp[2] * mTrans._32 + fTemp[5] * mTrans._33;
	tmp._21 = fTemp[1] * mTrans._30 + fTemp[6] * mTrans._32 + fTemp[9] * mTrans._33;
	tmp._21 -= fTemp[0] * mTrans._30 + fTemp[7] * mTrans._32 + fTemp[8] * mTrans._33;
	tmp._22 = fTemp[2] * mTrans._30 + fTemp[7] * mTrans._31 + fTemp[10] * mTrans._33;
	tmp._22 -= fTemp[3] * mTrans._30 + fTemp[6] * mTrans._31 + fTemp[11] * mTrans._33;
	tmp._23 = fTemp[5] * mTrans._30 + fTemp[8] * mTrans._31 + fTemp[11] * mTrans._32;
	tmp._23 -= fTemp[4] * mTrans._30 + fTemp[9] * mTrans._31 + fTemp[10] * mTrans._32;
	tmp._30 = fTemp[2] * mTrans._22 + fTemp[5] * mTrans._23 + fTemp[1] * mTrans._21;
	tmp._30 -= fTemp[4] * mTrans._23 + fTemp[0] * mTrans._21 + fTemp[3] * mTrans._22;
	tmp._31 = fTemp[8] * mTrans._23 + fTemp[0] * mTrans._20 + fTemp[7] * mTrans._22;
	tmp._31 -= fTemp[6] * mTrans._22 + fTemp[9] * mTrans._23 + fTemp[1] * mTrans._20;
	tmp._32 = fTemp[6] * mTrans._21 + fTemp[11] * mTrans._23 + fTemp[3] * mTrans._20;
	tmp._32 -= fTemp[10] * mTrans._23 + fTemp[2] * mTrans._20 + fTemp[7] * mTrans._21;
	tmp._33 = fTemp[10] * mTrans._22 + fTemp[4] * mTrans._20 + fTemp[9] * mTrans._21;
	tmp._33 -= fTemp[8] * mTrans._21 + fTemp[11] * mTrans._22 + fTemp[5] * mTrans._20;

	fDet = mTrans._00 * _00 + mTrans._01 * _01 + mTrans._02 * _02 + mTrans._03 * _03;

	fDet = 1 / fDet;

	tmp._00 *= fDet;
	tmp._01 *= fDet;
	tmp._02 *= fDet;
	tmp._03 *= fDet;

	tmp._10 *= fDet;
	tmp._11 *= fDet;
	tmp._12 *= fDet;
	tmp._13 *= fDet;

	tmp._20 *= fDet;
	tmp._21 *= fDet;
	tmp._22 *= fDet;
	tmp._23 *= fDet;

	tmp._30 *= fDet;
	tmp._31 *= fDet;
	tmp._32 *= fDet;
	tmp._33 *= fDet;

	return tmp;
}

FMatrix FMatrix::TranslateMatrix(const Vector3f& T)
{
	return FMatrix(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		T.x, T.y, T.z, 1.f);
}

FMatrix FMatrix::ScaleMatrix(float s)
{
	return FMatrix(
		s, 0.f, 0.f, 0.f,
		0.f, s, 0.f, 0.f,
		0.f, 0.f, s, 0.f,
		0.f, 0.f, 0.f, 1.f);
}

FMatrix FMatrix::RotateX(float v)
{
	float c = (float)cos(v);
	float s = (float)sin(v);
	return FMatrix(
		1.f, 0.f, 0.f, 0.f,
		0.f, c,   s,   0.f,
		0.f, -s,  c,   0.f,
		0.f, 0.f, 0.f, 1.f);
}

FMatrix FMatrix::RotateY(float v)
{
	float c = (float)cos(v);
	float s = (float)sin(v);
	return FMatrix(
		c,   0.f, -s, 0.f,
		0.f, 1.f, 0.f, 0.f,
		s,   0.f, c,   0.f,
		0.f, 0.f, 0.f, 1.f);
}

FMatrix FMatrix::RotateZ(float v)
{
	float c = (float)cos(v);
	float s = (float)sin(v);
	return FMatrix(
		c,   s,   0.f, 0.f,
		-s,  c,   0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
}

FMatrix FMatrix::MatrixRotationRollPitchYaw(float Roll, float Pitch, float Yaw)
{
	// roll(z), pitch(x), yaw(y)
	return FMatrix::RotateZ(Roll) * FMatrix::RotateX(Pitch) * FMatrix::RotateY(Yaw);
}

FMatrix FMatrix::MatrixLookAtLH(const Vector3f& EyePosition, const Vector3f& FocusPosition, const Vector3f& UpDirection)
{
	Vector3f Forward = FocusPosition - EyePosition;
	Forward = Forward.Normalize();
	Vector3f Up = UpDirection.Normalize();

	Vector3f Right = Cross(UpDirection, Forward);
	Up = Cross(Forward, Right);

	float D0 = -EyePosition.Dot(Right);
	float D1 = -EyePosition.Dot(Up);
	float D2 = -EyePosition.Dot(Forward);

	FMatrix Result(Vector4f(Right, D0), Vector4f(Up, D1), Vector4f(Forward, D2), Vector4f(0.f, 0.f, 0.f, 1.f));
	return Result.Transpose();
}



FMatrix FMatrix::MatrixPerspectiveFovLH(float FovAngleY, float AspectHByW, float NearZ, float FarZ)
{
	float h = 1.f / (float)tan(FovAngleY/2);
	float w = h / AspectHByW;
	return FMatrix(
		w, 0.f, 0.f, 0.f,
		0.f, h, 0.f, 0.f,
		0.f, 0.f, FarZ/(FarZ-NearZ), 1,
		0.f, 0.f, -NearZ*FarZ/(FarZ-NearZ), 0.f
	);
}

FMatrix FMatrix::MatrixOrthoLH(float Width, float Height, float NearZ, float FarZ)
{
	float Dist = FarZ - NearZ;
	return FMatrix(
		2/Width, 0, 0, 0,
		0, 2/Height, 0, 0,
		0, 0, 1/Dist, 0,
		0, 0, -NearZ/Dist, 1
);
}

FMatrix FMatrix::MatrixOrthographicOffCenterLH(float ViewLeft, float ViewRight, float ViewBottom, float ViewTop, float NearZ, float FarZ)
{
	float ReciprocalWidth = 1.0f / (ViewRight - ViewLeft);
	float ReciprocalHeight = 1.0f / (ViewTop - ViewBottom);
	float fRange = 1.0f / (FarZ - NearZ);
	return FMatrix(
		ReciprocalWidth + ReciprocalWidth, 0, 0, 0,
		0, ReciprocalHeight + ReciprocalHeight, 0, 0,
		0, 0, fRange, 0,
		-(ViewLeft + ViewRight) * ReciprocalWidth, -(ViewTop + ViewBottom) * ReciprocalHeight, -fRange * NearZ, 1
	);
}

//Vector4f operator*(const FMatrix& mat, const Vector4f& vec)
//{
//	return Vector4f();
//}

Vector4f operator*(const Vector4f& vec, const FMatrix& mat)
{
	float d0 = vec.Dot(mat.Column(0));
	float d1 = vec.Dot(mat.Column(1));
	float d2 = vec.Dot(mat.Column(2));
	float d3 = vec.Dot(mat.Column(3));
	return Vector4f(d0, d1, d2, d3);
}

