#pragma once
#include "tiny_gltf.h"
#include "MathLib.h"

/****************************************************
*	Node结构体
*	每个Node需要包含：
*		Mesh 变换矩阵
****************************************************/
struct F3DNodeInfo
{
	F3DNodeInfo()
	{
		Rotation = Vector4f(0, 0, 0, 1);
		Scale = Vector3f(1, 1, 1);
		Translate = Vector3f(0, 0, 0);

		MeshID = -1;
		SkinID = -1;
		NodeID = -1;
		ParentNode ;
	}
	~F3DNodeInfo()
	{
		ChildrenNode.clear();
	}

	Vector4f Rotation;
	Vector3f Scale;
	Vector3f Translate;

	int MeshID;
	int SkinID;

	int NodeID;
	std::weak_ptr<F3DNodeInfo> ParentNode;
	std::vector<std::shared_ptr<F3DNodeInfo>> ChildrenNode;

	FMatrix FinalMeshMat;
};

class FGltfNode
{
public:
	FGltfNode(tinygltf::Model* Model);
	~FGltfNode();

	void InitGroupNode(uint32_t nodeIndex);
	void UpdateNode();
	void UpdateNodeParent(std::shared_ptr<F3DNodeInfo> NodeInfo);
	const std::vector<std::shared_ptr<F3DNodeInfo>>& GetAllNodes() const;
private:
	void InitNode();
	void CreateModelNodeTree(std::shared_ptr<F3DNodeInfo> NodeInfo);
	void DFSNodeTree(std::shared_ptr<F3DNodeInfo> NodeInfo, FMatrix& ParentMatrix);
private:
	tinygltf::Model* m_Model;

	std::vector<std::shared_ptr<F3DNodeInfo>> m_Node;
	std::vector<std::shared_ptr<F3DNodeInfo>> m_GroupNode;
};