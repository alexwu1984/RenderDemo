#include "GltfNode.h"

FGltfNode::FGltfNode(tinygltf::Model* Model)
	:m_Model(Model)
{
	InitNode();
}

FGltfNode::~FGltfNode()
{

}

void FGltfNode::InitGroupNode(uint32_t nodeIndex)
{
	if (nodeIndex >= 0 && nodeIndex < m_Model->nodes.size())
	{
		std::shared_ptr<F3DNodeInfo> GroupNode = m_Node[nodeIndex];

		auto& child = m_Model->nodes[nodeIndex].children;
		if (child.size() > 0)
		{
			CreateModelNodeTree(GroupNode);

			FMatrix Identity;

			DFSNodeTree(GroupNode, Identity);

			m_GroupNode.push_back(GroupNode);
		}


	}
}

void FGltfNode::UpdateNode()
{
	for (int i = 0; i < m_GroupNode.size(); i++)
	{
		FMatrix Identity;

		DFSNodeTree(m_GroupNode[i], Identity);
	}
}

void FGltfNode::UpdateNodeParent(std::shared_ptr<F3DNodeInfo> NodeInfo)
{
	FMatrix mat4Scaling = FMatrix::ScaleMatrix(Vector3f(NodeInfo->Scale.x, NodeInfo->Scale.y, NodeInfo->Scale.z));
	FMatrix mat4Rotation = MathLib::QuaternionToMatrix(NodeInfo->Rotation);
	FMatrix mat4Translation = FMatrix::TranslateMatrix(Vector3f(NodeInfo->Translate.x, NodeInfo->Translate.y, NodeInfo->Translate.z));

	FMatrix NodeTransformation = mat4Scaling * mat4Rotation * mat4Translation;
	if (NodeInfo->ParentNode.lock() )
	{
		UpdateNodeParent(NodeInfo->ParentNode.lock());
		NodeInfo->FinalMeshMat = NodeInfo->ParentNode.lock()->FinalMeshMat * NodeTransformation;
	}
	else
	{
		NodeInfo->FinalMeshMat = NodeTransformation;
	}
}

const std::vector<std::shared_ptr<F3DNodeInfo>>& FGltfNode::GetAllNodes() const
{
	return m_Node;
}

void FGltfNode::InitNode()
{
	const auto& Nodes = m_Model->nodes;

	for (int i = 0; i < Nodes.size(); ++i)
	{
		const auto& Node = Nodes[i];

		std::shared_ptr< F3DNodeInfo> NodeInfo = std::make_shared<F3DNodeInfo>();
		NodeInfo->MeshID = Node.mesh;
		NodeInfo->SkinID = Node.skin;
		NodeInfo->NodeID = i;

		FMatrix NodeMat;
		if (Node.matrix.size() == 16)
		{
			float* mat16 = &NodeMat._01;
			for (int m = 0; m < 16; m++)
			{
				mat16[m] = Node.matrix[m];
			}
		}

		if (Node.translation.size() == 3)
		{
			NodeMat *= FMatrix::TranslateMatrix(Vector3f(Node.translation[0], Node.translation[1], Node.translation[2]));
		}

		if (Node.scale.size() == 3)
		{
			NodeMat *= FMatrix::ScaleMatrix(Vector3f(Node.scale[0], Node.scale[1], Node.scale[2]));
			NodeInfo->Scale = Vector3f(Node.scale[0], Node.scale[1], Node.scale[2]);
		}

		if (Node.rotation.size() == 4)
		{
			NodeMat *= MathLib::QuaternionToMatrix(Vector4f(Node.rotation[0], Node.rotation[1], Node.rotation[2], Node.rotation[3]));
			NodeInfo->Rotation = Vector4f(Node.rotation[0], Node.rotation[1], Node.rotation[2], Node.rotation[3]);
		}

		NodeInfo->FinalMeshMat = NodeMat;
		
		m_Node.push_back(NodeInfo);
	}
}

void FGltfNode::CreateModelNodeTree(std::shared_ptr<F3DNodeInfo> NodeInfo)
{
	int NodeID = NodeInfo->NodeID;
	auto& child = m_Model->nodes[NodeID].children;
	for (int i = 0; i < child.size(); i++)
	{
		int nodeId = child[i];
		std::shared_ptr<F3DNodeInfo> pChild = m_Node[nodeId];
		pChild->ParentNode = NodeInfo;
		NodeInfo->ChildrenNode.push_back(pChild);

		CreateModelNodeTree(pChild);
	}
}

void FGltfNode::DFSNodeTree(std::shared_ptr<F3DNodeInfo> NodeInfo, FMatrix& ParentMatrix)
{
	FMatrix NodeTransformation = NodeInfo->FinalMeshMat * ParentMatrix;

	NodeInfo->FinalMeshMat = NodeTransformation;

	for (int i = 0; i < NodeInfo->ChildrenNode.size(); i++)
	{
		DFSNodeTree(NodeInfo->ChildrenNode[i], NodeTransformation);
	}
}
