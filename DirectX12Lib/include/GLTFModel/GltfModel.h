#pragma once
#include "tiny_gltf.h"
#include <memory>
#include "MathLib.h"

class FGltfMaterial;
class FGltfNode;
class FGltfMesh;

class FGLTFMode
{
public:
	FGLTFMode(const std::wstring& FileName);
	FGLTFMode();
	~FGLTFMode();

	const std::vector<std::shared_ptr<FGltfMesh>>& GetModelMesh() const;

private:
	void LoadNode();
	std::vector <std::shared_ptr<FGltfMaterial>> LoadMaterial();
	void LoadMesh();

private:
	tinygltf::TinyGLTF m_GltfCtx ;
	tinygltf::Model m_GltfMode;
	std::shared_ptr<FGltfNode> m_ModelNode;
	std::vector<std::shared_ptr<FGltfMesh>> m_ModelMesh;

	FBoundingBox m_ModelBox;
};