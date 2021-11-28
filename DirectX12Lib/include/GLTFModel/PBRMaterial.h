#pragma once
#include "GltfMaterial.h"

class FTexture;


__declspec(align(16)) struct FPBRPSConstants
{
	float		Exposure = 1.0;
	Vector3f	CameraPos;
	Vector3f	BaseColor;
	float		Metallic = 1.0;
	float		Roughness = 0.0;
	int			MaxMipLevel;
	int			bSHDiffuse = false;
	int			Degree = 0;
	FMatrix		InvViewProj;
	Vector4f	TemporalAAJitter;
	Vector4f	Coeffs[16];
	Vector3f	LightDir;
	int			EnableLight = 0;
	int         EnableSSR = 0;
};


class FPBRMaterial : public FGltfMaterial
{
public:
	FPBRMaterial(tinygltf::Model* Model);
	~FPBRMaterial();

	FPBRPSConstants& GetPS() { return m_PS; }

	virtual void InitTexture(const tinygltf::Material& material);
	std::shared_ptr<FTexture> GetBaseColorTexture() const;
	std::shared_ptr<FTexture> GetMetallicRoughnessTexture() const;
	std::shared_ptr<FTexture> GetNormalTexture() const;
	std::shared_ptr<FTexture> GetEmissiveTexture() const;
	std::shared_ptr<FTexture> GetOcclusionTexture() const;

public:
	FPBRPSConstants m_PS;

protected:
	std::shared_ptr<FTexture> m_BaseColorTexture;
	std::shared_ptr<FTexture> m_MetallicRoughnessTexture;
	std::shared_ptr<FTexture> m_NormalTexture;
	std::shared_ptr<FTexture> m_EmissiveTexture;
	std::shared_ptr<FTexture> m_OcclusionTexture;
};