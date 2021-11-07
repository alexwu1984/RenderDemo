#include "GltfMaterial.h"

FGltfMaterial::FGltfMaterial(tinygltf::Model* Model)
	:m_Model(Model)
{

}

FGltfMaterial::~FGltfMaterial()
{

}

void FGltfMaterial::InitMaterial(uint32_t MaterialIndex)
{
	const tinygltf::Material& material = m_Model->materials[MaterialIndex];

	m_MaterialName = material.name;
	m_AlphaMode = material.alphaMode;
	m_DoubleSided = material.doubleSided;
	m_AlphaCutoff = material.alphaCutoff;
	m_MetallicFactor = material.pbrMetallicRoughness.metallicFactor;
	m_RoughnessFactor = material.pbrMetallicRoughness.roughnessFactor;

	InitTexture(material);
	
}

std::string FGltfMaterial::GetMaerialName() const
{
	return m_MaterialName;
}

std::string FGltfMaterial::GetAlphaMode() const
{
	return m_AlphaMode;
}
