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
	void LoadDiffuseTexture(const std::wstring& file);
	const FTexture& GetDiffuseTexture()const { return m_DiffuseTex; }
	void LoadNormalTexture(const std::wstring& file);
	const FTexture& GetNormalTexture() const { return m_NormalTex; }
	void LoadSpecularTexture(const std::wstring& file);
	const FTexture& GetSpecularTexture() const { return m_SpecularTex; }
	void LoadEmissiveTexture(const std::wstring& file);
	const FTexture& GetEmissiveTexture() const { return m_EmissiveTex;}
	void LoadAmbientTexture(const std::wstring& file);
	const FTexture& GetAmbientTexture() { return m_AmbientTex; }
	void LoadAlphaTexture(const std::wstring& file);
	const FTexture& GetAlphaTexture()const { return m_AlphaTex; }

	std::tuple<Vector3f, Vector3f>  GetColor() const { return { m_colorInfo.kd,m_colorInfo.ks }; }

private:
	FTexture m_DiffuseTex;
	FTexture m_NormalTex;
	FTexture m_SpecularTex;
	FTexture m_EmissiveTex;
	FTexture m_AmbientTex;
	FTexture m_AlphaTex;

	struct ColorInfo
	{
		Vector3f kd;
		Vector3f ks;
	};
	ColorInfo m_colorInfo;
};