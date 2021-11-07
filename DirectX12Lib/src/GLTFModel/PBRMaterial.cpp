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
		texture2d->Create(ModelImage.width, ModelImage.height, DXGI_FORMAT_B8G8R8A8_UNORM, pData);
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

	index = material.emissiveTexture.index;
	if (index > -1 && index < gltfTexture.size())
	{
		m_EmissiveTexture = CreateTexture(gltfTexture[index].source);
	}

	index = material.normalTexture.index;
	if (index > -1 && index < gltfTexture.size())
	{
		m_NormalTexture = CreateTexture(gltfTexture[index].source);
	}

	index = material.occlusionTexture.index;
	if (index > -1 && index < gltfTexture.size())
	{
		m_OcclusionTexture = CreateTexture(gltfTexture[index].source);
	}
}
