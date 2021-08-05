#include "Material.h"

FMaterial::FMaterial()
{

}

FMaterial::~FMaterial()
{

}


void FMaterial::SetColor(const Vector3f& kd, const Vector3f& ks)
{
	m_colorInfo.kd = kd;
	m_colorInfo.ks = ks;
}

void FMaterial::LoadDiffuseTexture(const std::wstring& file)
{
	m_DiffuseTex.LoadFromFile(file);
}

void FMaterial::LoadNormalTexture(const std::wstring& file)
{
	m_NormalTex.LoadFromFile(file);
}

void FMaterial::LoadRoughnessPath(const std::wstring& file)
{
	m_RoughnessTex.LoadFromFile(file);
}

void FMaterial::LoadEmissiveTexture(const std::wstring& file)
{
	m_EmissiveTex.LoadFromFile(file);
}

void FMaterial::LoadAmbientTexture(const std::wstring& file)
{
	m_AmbientTex.LoadFromFile(file);
}

void FMaterial::LoadOpacityTexture(const std::wstring& file)
{
	m_OpacityTex.LoadFromFile(file);
}

void FMaterial::LoadMetallicTexture(const std::wstring& file)
{
	m_MetallicTex.LoadFromFile(file);
}
