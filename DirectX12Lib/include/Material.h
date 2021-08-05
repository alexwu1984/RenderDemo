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
	void LoadRoughnessPath(const std::wstring& file);
	const FTexture& GetRoughnessTexture() const { return m_RoughnessTex; }
	void LoadEmissiveTexture(const std::wstring& file);
	const FTexture& GetEmissiveTexture() const { return m_EmissiveTex;}
	void LoadAmbientTexture(const std::wstring& file);
	const FTexture& GetAmbientTexture() { return m_AmbientTex; }
	void LoadOpacityTexture(const std::wstring& file);
	const FTexture& GetOpacityTexture()const { return m_OpacityTex; }
	void LoadMetallicTexture(const std::wstring& file);
	const FTexture& GetMetallicTexture()const { return m_MetallicTex; }

	std::tuple<Vector3f, Vector3f>  GetColor() const { return { m_colorInfo.kd,m_colorInfo.ks }; }

private:
	FTexture m_DiffuseTex;
	FTexture m_NormalTex;
	FTexture m_RoughnessTex;
	FTexture m_EmissiveTex;
	FTexture m_AmbientTex;
	FTexture m_OpacityTex;
	FTexture m_MetallicTex;

	struct ColorInfo
	{
		Vector3f kd;
		Vector3f ks;
	};
	ColorInfo m_colorInfo;
};