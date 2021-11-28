#include "PBRMaterial.h"

FPBRMaterial::FPBRMaterial(tinygltf::Model* Model)
	:FGltfMaterial(Model)
{

}

FPBRMaterial::~FPBRMaterial()
{

}

void FPBRMaterial::InitTexture(const tinygltf::Material& material)
{
	auto& gltfTexture = m_Model->textures;

	auto CreateTexture = [this](int32_t index)->std::shared_ptr<FTexture> {
		auto& ModelImage = m_Model->images[index];
		uint8_t* pData = (uint8_t*)ModelImage.image.data();
		std::shared_ptr<FTexture> texture2d = std::make_shared<FTexture>();
		texture2d->Create(ModelImage.width, ModelImage.height, DXGI_FORMAT_R8G8B8A8_UNORM, pData);
		return texture2d;
	};

	int32_t index = material.pbrMetallicRoughness.baseColorTexture.index;
	if (index > -1 && index < gltfTexture.size() )
	{
		m_BaseColorTexture = CreateTexture(gltfTexture[index].source);
	}

	index = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
	if (index > -1 && index < gltfTexture.size())
	{
		m_MetallicRoughnessTexture = CreateTexture(gltfTexture[index].source);
	}
	else
	{
		std::shared_ptr<FTexture> texture2d = std::make_shared<FTexture>();
		texture2d->Create(1.0, material.pbrMetallicRoughness.roughnessFactor, material.pbrMetallicRoughness.metallicFactor, 1.0);
		m_MetallicRoughnessTexture = texture2d;
	}


	index = material.emissiveTexture.index;
	if (index > -1 && index < gltfTexture.size())
	{
		m_EmissiveTexture = CreateTexture(gltfTexture[index].source);
	}
	else
	{
		auto emissiveColor = material.emissiveFactor;
		std::shared_ptr<FTexture> texture2d = std::make_shared<FTexture>();
		texture2d->Create((float)emissiveColor[0], (float)emissiveColor[1], (float)emissiveColor[2], 1.0);
		m_EmissiveTexture = texture2d;
	}

	index = material.normalTexture.index;
	if (index > -1 && index < gltfTexture.size())
	{
		m_NormalTexture = CreateTexture(gltfTexture[index].source);
	}
	else
	{
		std::shared_ptr<FTexture> texture2d = std::make_shared<FTexture>();
		texture2d->Create(0.5f, 0.5f, 1.0f, 1.0f);
		m_NormalTexture = texture2d;
	}

	index = material.occlusionTexture.index;
	if (index > -1 && index < gltfTexture.size())
	{
		m_OcclusionTexture = CreateTexture(gltfTexture[index].source);
	}
	else
	{
		std::shared_ptr<FTexture> texture2d = std::make_shared<FTexture>();
		texture2d->Create(1.0f, 1.0f, 1.0f, 1.0f);
		m_OcclusionTexture = texture2d;
	}
}

std::shared_ptr<FTexture> FPBRMaterial::GetBaseColorTexture() const
{
	return m_BaseColorTexture;
}

std::shared_ptr<FTexture> FPBRMaterial::GetMetallicRoughnessTexture() const
{
	return m_MetallicRoughnessTexture;
}

std::shared_ptr<FTexture> FPBRMaterial::GetNormalTexture() const
{
	return m_NormalTexture;
}

std::shared_ptr<FTexture> FPBRMaterial::GetEmissiveTexture() const
{
	return m_EmissiveTexture;
}

std::shared_ptr<FTexture> FPBRMaterial::GetOcclusionTexture() const
{
	return m_OcclusionTexture;
}
