#pragma once
#include "Material.h"
#include "tiny_gltf.h"

class FGltfMaterial
{

public:
	FGltfMaterial(tinygltf::Model* Model);
	~FGltfMaterial();

	void  InitMaterial(uint32_t MaterialIndex);
	virtual void InitTexture(const tinygltf::Material& material) = 0;
	std::string GetMaerialName() const;
	std::string GetAlphaMode() const;
protected:
	tinygltf::Model* m_Model = nullptr;
	std::string m_MaterialName;
	std::string m_AlphaMode;
	float m_AlphaCutoff = 0;
	bool m_DoubleSided = false;
	float m_MetallicFactor = 0;
	float m_RoughnessFactor = 0;
};