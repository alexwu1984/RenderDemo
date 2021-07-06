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

void FMaterial::LoadTexture(const std::wstring& file)
{
	m_Texture.LoadFromFile(file);
}
