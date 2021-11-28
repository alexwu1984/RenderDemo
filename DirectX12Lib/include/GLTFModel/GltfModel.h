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

	void SetScale(float Scale);
	void SetRotation(const FMatrix& Rotation);
	const FMatrix GetModelMatrix() { return m_ModelMatrix; }
	const FMatrix GetPreviousModelMatrix() { return m_PreviousModelMatrix; }
	void Update();

	const std::vector<std::shared_ptr<FGltfMesh>>& GetModelMesh() const;

private:
	void LoadNode();
	std::vector <std::shared_ptr<FGltfMaterial>> LoadMaterial();
	void LoadMesh();
	void UpdateModelMatrix();

private:
	tinygltf::TinyGLTF m_GltfCtx ;
	tinygltf::Model m_GltfMode;
	std::shared_ptr<FGltfNode> m_ModelNode;
	std::vector<std::shared_ptr<FGltfMesh>> m_ModelMesh;

	FBoundingBox m_ModelBox;

	float m_Scale;
	FMatrix m_RotationMatrix;
	FMatrix m_ModelMatrix;
	FMatrix m_PreviousModelMatrix;
};