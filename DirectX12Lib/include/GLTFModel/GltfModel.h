#pragma once
#include "tiny_gltf.h"
#include <memory>
#include "MathLib.h"

class FGLTFMode
{
	friend struct FRenderItem;
public:
	FGLTFMode(const std::wstring& FileName);
	FGLTFMode();
	~FGLTFMode();

private:
	void LoadNode();
	void LoadMaterial();
	void LoadMesh();

private:
	tinygltf::TinyGLTF m_GltfCtx ;
	tinygltf::Model m_GltfMode;
	std::shared_ptr<class FGltfNode> m_ModelNode;
	std::vector < std::shared_ptr<class FGltfMaterial>> m_ModelMaterial;
	std::vector< std::shared_ptr<class FGltfMesh>> m_ModelMesh;

	FBoundingBox m_ModelBox;
};