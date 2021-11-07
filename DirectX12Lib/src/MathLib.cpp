#include "MathLib.h"
#include "Common.h"

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

FMatrix FMatrix::operator * (float rhs) const
{
	return FMatrix(r0 * rhs, r1 * rhs, r2 * rhs, r3 * rhs);
}

FMatrix& FMatrix::operator*=(float rhs)
{
	*this = (*this) * rhs;
	return *this;
}

Vector4f FMatrix::operator [](int index) const
{
	Assert(index < 4);
	return row[index];
}
Vector4f& FMatrix::operator[](int index)
{
	Assert(index < 4);
	return row[index];
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

FMatrix FMatrix::Transpose() const
{
	return FMatrix(
		r0.x, r1.x, r2.x, r3.x,
		r0.y, r1.y, r2.y, r3.y,
		r0.z, r1.z, r2.z, r3.z,
		r0.w, r1.w, r2.w, r3.w
	);
}

FMatrix FMatrix::Inverse() const
{
	// https://semath.info/src/inverse-cofactor-ex4.html
	float det = r0.x * r1.y * r2.z * r3.w + r0.x * r1.z * r2.w * r3.y + r0.x * r1.w * r2.y * r3.z
		- r0.x * r1.w * r2.z * r3.y - r0.x * r1.z * r2.y * r3.w - r0.x * r1.y * r2.w * r3.z
		- r0.y * r1.x * r2.z * r3.w - r0.z * r1.x * r2.w * r3.y - r0.w * r1.x * r2.y * r3.z
		+ r0.w * r1.x * r2.z * r3.y + r0.z * r1.x * r2.y * r3.w + r0.y * r1.x * r2.w * r3.z
		+ r0.y * r1.z * r2.x * r3.w + r0.z * r1.w * r2.x * r3.y + r0.w * r1.y * r2.x * r3.z
		- r0.w * r1.z * r2.x * r3.y - r0.z * r1.y * r2.x * r3.w - r0.y * r1.w * r2.x * r3.z
		- r0.y * r1.z * r2.w * r3.x - r0.z * r1.w * r2.y * r3.x - r0.w * r1.y * r2.z * r3.x
		+ r0.w * r1.z * r2.y * r3.x + r0.z * r1.y * r2.w * r3.x + r0.y * r1.w * r2.z * r3.x;

	float A11 = r1.y * r2.z * r3.w + r1.z * r2.w * r3.y + r1.w * r2.y * r3.z - r1.w * r2.z * r3.y - r1.z * r2.y * r3.w - r1.y * r2.w * r3.z;
	float A12 = -r0.y * r2.z * r3.w - r0.z * r2.w * r3.y - r0.w * r2.y * r3.z + r0.w * r2.z * r3.y + r0.z * r2.y * r3.w + r0.y * r2.w * r3.z;
	float A13 = r0.y * r1.z * r3.w + r0.z * r1.w * r3.y + r0.w * r1.y * r3.z - r0.w * r1.z * r3.y - r0.z * r1.y * r3.w - r0.y * r1.w * r3.z;
	float A14 = -r0.y * r1.z * r2.w - r0.z * r1.w * r2.y - r0.w * r1.y * r2.z + r0.w * r1.z * r2.y + r0.z * r1.y * r2.w + r0.y * r1.w * r2.z;

	float A21 = -r1.x * r2.z * r3.w - r1.z * r2.w * r3.x - r1.w * r2.x * r3.z + r1.w * r2.z * r3.x + r1.z * r2.x * r3.w + r1.x * r2.w * r3.z;
	float A22 = r0.x * r2.z * r3.w + r0.z * r2.w * r3.x + r0.w * r2.x * r3.z - r0.w * r2.z * r3.x - r0.z * r2.x * r3.w - r0.x * r2.w * r3.z;
	float A23 = -r0.x * r1.z * r3.w - r0.z * r1.w * r3.x - r0.w * r1.x * r3.z + r0.w * r1.z * r3.x + r0.z * r1.x * r3.w + r0.x * r1.w * r3.z;
	float A24 = r0.x * r1.z * r2.w + r0.z * r1.w * r2.x + r0.w * r1.x * r2.z - r0.w * r1.z * r2.x - r0.z * r1.x * r2.w - r0.x * r1.w * r2.z;

	float A31 = r1.x * r2.y * r3.w + r1.y * r2.w * r3.x + r1.w * r2.x * r3.y - r1.w * r2.y * r3.x - r1.y * r2.x * r3.w - r1.x * r2.w * r3.y;
	float A32 = -r0.x * r2.y * r3.w - r0.y * r2.w * r3.x - r0.w * r2.x * r3.y + r0.w * r2.y * r3.x + r0.y * r2.x * r3.w + r0.x * r2.w * r3.y;
	float A33 = r0.x * r1.y * r3.w + r0.y * r1.w * r3.x + r0.w * r1.x * r3.y - r0.w * r1.y * r3.x - r0.y * r1.x * r3.w - r0.x * r1.w * r3.y;
	float A34 = -r0.x * r1.y * r2.w - r0.y * r1.w * r2.x - r0.w * r1.x * r2.y + r0.w * r1.y * r2.x + r0.y * r1.x * r2.w + r0.x * r1.w * r2.y;

	float A41 = -r1.x * r2.y * r3.z - r1.y * r2.z * r3.x - r1.z * r2.x * r3.y + r1.z * r2.y * r3.x + r1.y * r2.x * r3.z + r1.x * r2.z * r3.y;
	float A42 = r0.x * r2.y * r3.z + r0.y * r2.z * r3.x + r0.z * r2.x * r3.y - r0.z * r2.y * r3.x - r0.y * r2.x * r3.z - r0.x * r2.z * r3.y;
	float A43 = -r0.x * r1.y * r3.z - r0.y * r1.z * r3.x - r0.z * r1.x * r3.y + r0.z * r1.y * r3.x + r0.y * r1.x * r3.z + r0.x * r1.z * r3.y;
	float A44 = r0.x * r1.y * r2.z + r0.y * r1.z * r2.x + r0.z * r1.x * r2.y - r0.z * r1.y * r2.x - r0.y * r1.x * r2.z - r0.x * r1.z * r2.y;

	return FMatrix(A11, A12, A13, A14, A21, A22, A23, A24, A31, A32, A33, A34, A41, A42, A43, A44) * (1.f / det);
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

FMatrix FMatrix::ScaleMatrix(const Vector3f& T)
{
	return FMatrix(
		T.x, 0.f, 0.f, 0.f,
		0.f, T.y, 0.f, 0.f,
		0.f, 0.f, T.z, 0.f,
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

void MathLib::QuaternionNormalize(Vector4f& vec4Out)
{
	const float mag = std::sqrt(vec4Out.x * vec4Out.x + vec4Out.y * vec4Out.y + vec4Out.z * vec4Out.z + vec4Out.w * vec4Out.w);
	if (mag)
	{
		const float invMag = 1.0f / mag;
		vec4Out.x *= invMag;
		vec4Out.y *= invMag;
		vec4Out.z *= invMag;
		vec4Out.w *= invMag;
	}
}

void MathLib::QuaternionInterpolate(Vector4f& vec4Out, const Vector4f& vec4Start, const Vector4f& vec4End, float fFactor)
{
	// calc cosine theta
	float cosom = vec4Start.x * vec4End.x + vec4Start.y * vec4End.y + vec4Start.z * vec4End.z + vec4Start.w * vec4End.w;

	// adjust signs (if necessary)
	Vector4f  end = vec4End;
	if (cosom < 0.0f)
	{
		cosom = -cosom;
		end.x = -end.x;   // Reverse all signs
		end.y = -end.y;
		end.z = -end.z;
		end.w = -end.w;
	}

	// Calculate coefficients
	float sclp, sclq;
	if ((1.0f - cosom) > 0.0001f) // 0.0001 -> some epsillon
	{
		// Standard case (slerp)
		float omega, sinom;
		omega = (std::acos)(cosom); // extract theta from dot product's cos theta
		sinom = (std::sin)(omega);
		sclp = (std::sin)((1.0f - fFactor) * omega) / sinom;
		sclq = (std::sin)(fFactor * omega) / sinom;
	}
	else
	{
		// Very close, do linear interp (because it's faster)
		sclp = 1.0f - fFactor;
		sclq = fFactor;
	}

	vec4Out.x = sclp * vec4Start.x + sclq * end.x;
	vec4Out.y = sclp * vec4Start.y + sclq * end.y;
	vec4Out.z = sclp * vec4Start.z + sclq * end.z;
	vec4Out.w = sclp * vec4Start.w + sclq * end.w;
}

FMatrix MathLib::QuaternionToMatrix(const Vector4f& vec4InQuaternion)
{
	FMatrix resMatrix;
	Vector4 tmp = vec4InQuaternion;
	QuaternionNormalize(tmp);
	float x = tmp.x;
	float y = tmp.y;
	float z = tmp.z;
	float w = tmp.w;

	resMatrix[0][0] = (1.0f) - (2.0f) * (y * y + z * z);
	resMatrix[1][0] = (2.0f) * (x * y - z * w);
	resMatrix[2][0] = (2.0f) * (x * z + y * w);
	resMatrix[3][0] = 0.0f;

	resMatrix[0][1] = (2.0f) * (x * y + z * w);
	resMatrix[1][1] = (1.0f) - (2.0f) * (x * x + z * z);
	resMatrix[2][1] = (2.0f) * (y * z - x * w);
	resMatrix[3][1] = 0.0f;

	resMatrix[0][2] = (2.0f) * (x * z - y * w);
	resMatrix[1][2] = (2.0f) * (y * z + x * w);
	resMatrix[2][2] = (1.0f) - (2.0f) * (x * x + y * y);
	resMatrix[3][2] = 0.0f;

	resMatrix[0][3] = 0.0f;
	resMatrix[1][3] = 0.0f;
	resMatrix[2][3] = 0.0f;
	resMatrix[3][3] = 1.0f;

	return FMatrix(resMatrix.r0,resMatrix.r1,resMatrix.r2,resMatrix.r3);
}
