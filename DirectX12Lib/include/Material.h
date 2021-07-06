#pragma once
#include "Texture.h"
#include "MathLib.h"
#include <tuple>


class FMaterial
{
public:
	FMaterial();
	~FMaterial();

	void SetColor(const Vector3f& kd, const Vector3f& ks);
	void LoadTexture(const std::wstring& file);
	const FTexture& GetTexture()const { return m_Texture; }

	std::tuple<Vector3f, Vector3f>  GetColor() const { return { m_colorInfo.kd,m_colorInfo.ks }; }

private:
	FTexture m_Texture;

	struct ColorInfo
	{
		Vector3f kd;
		Vector3f ks;
	};
	ColorInfo m_colorInfo;
};