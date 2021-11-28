#include "GltfModel.h"
#include "StringUnit.h"
#include "GltfNode.h"
#include "Texture.h"
#include "GltfMaterial.h"
#include "PBRMaterial.h"
#include "GltfMesh.h"

FGLTFMode::FGLTFMode(const std::wstring& FileName)
{
	std::string err;
	std::string warn;
	std::string utf8FileName = core::ucs2_u8(FileName);

	if (FileName.find(L".gltf") != std::wstring::npos)
	{
		if (m_GltfCtx.LoadASCIIFromFile(&m_GltfMode, &err, &warn, utf8FileName))
		{
			LoadNode();
			LoadMesh();
		}
	}
	else
	{
		if (m_GltfCtx.LoadBinaryFromFile(&m_GltfMode, &err, &warn, utf8FileName))
		{
			LoadNode();
			LoadMesh();
		}
	}

}

FGLTFMode::FGLTFMode()
{

}

FGLTFMode::~FGLTFMode()
{

}

void FGLTFMode::SetScale(float Scale)
{
	m_Scale = Scale;
	UpdateModelMatrix();
}

void FGLTFMode::SetRotation(const FMatrix& Rotation)
{
	m_RotationMatrix = Rotation;
	UpdateModelMatrix();
}

void FGLTFMode::Update()
{
	m_PreviousModelMatrix = m_ModelMatrix;
}

const std::vector<std::shared_ptr<FGltfMesh>>& FGLTFMode::GetModelMesh() const
{
	return m_ModelMesh;
}

void FGLTFMode::LoadNode()
{
	if (!m_GltfMode.nodes.empty())
	{
		std::shared_ptr<FGltfNode> RootNode = std::make_shared<FGltfNode>(&m_GltfMode);
		m_ModelNode = RootNode;

		auto& Scenes = m_GltfMode.scenes;
		if (Scenes.size() > 0)
		{
			auto& NodeIds = Scenes[0].nodes;
			for (int k = 0; k < NodeIds.size(); k++)
			{
				RootNode->InitGroupNode(NodeIds[k]);
			}
		}
	}
}


std::vector <std::shared_ptr<FGltfMaterial>> FGLTFMode::LoadMaterial()
{
	std::vector <std::shared_ptr<FGltfMaterial>> ModelMaterial;
	for (int i = 0; i < m_GltfMode.materials.size(); i++)
	{
		std::shared_ptr< FPBRMaterial> PBRMaterial = std::make_shared<FPBRMaterial>(&m_GltfMode);
		PBRMaterial->InitMaterial(i);
		ModelMaterial.push_back(PBRMaterial);
	}
	return ModelMaterial;
}

void FGLTFMode::LoadMesh()
{
	std::vector <std::shared_ptr<FGltfMaterial>> ModelMaterial(std::move(LoadMaterial()));
	for (int i = 0; i < m_GltfMode.meshes.size(); i++)
	{
		auto& ModelMesh = m_GltfMode.meshes[i];
		for (int j = 0; j < ModelMesh.primitives.size(); j++)
		{
			std::shared_ptr<FGltfMesh> Mesh = std::make_shared<FGltfMesh>(&m_GltfMode);
			Mesh->Init(i, j, ModelMaterial, m_ModelNode);

			//if (pMesh->m_mesh->pBoneWeights != NULL)
			//{
			//	m_hasSkin = true;
			//}
			m_ModelMesh.push_back(Mesh);
		}
	}

	bool isInit = false;
	for (int i = 0; i < m_ModelMesh.size(); i++)
	{
		const FBoundingBox& MeshBox = m_ModelMesh[i]->GetBoundingBox();
		Vector4f tmp = Vector4f(MeshBox.minPoint.x, MeshBox.minPoint.y, MeshBox.minPoint.z, 1.0);
		Vector4f tmp2 = Vector4f(MeshBox.maxPoint.x, MeshBox.maxPoint.y, MeshBox.maxPoint.z, 1.0);

		tmp = tmp * m_ModelMesh[i]->GetMeshMat();
		tmp = tmp / tmp.w;

		tmp2 = tmp2  * m_ModelMesh[i]->GetMeshMat();
		tmp2 = tmp2 / tmp2.w;


		if (!isInit)
		{
			m_ModelBox.minPoint.x = tmp.x;
			m_ModelBox.minPoint.y = tmp.y;
			m_ModelBox.minPoint.z = tmp.z;
			m_ModelBox.maxPoint.x = tmp.x;
			m_ModelBox.maxPoint.y = tmp.y;
			m_ModelBox.maxPoint.z = tmp.z;
			isInit = true;
		}


		m_ModelBox.minPoint.x = (std::min)(m_ModelBox.minPoint.x, tmp.x);
		m_ModelBox.minPoint.y = (std::min)(m_ModelBox.minPoint.y, tmp.y);
		m_ModelBox.minPoint.z = (std::min)(m_ModelBox.minPoint.z, tmp.z);

		m_ModelBox.maxPoint.x = (std::max)(m_ModelBox.maxPoint.x, tmp.x);
		m_ModelBox.maxPoint.y = (std::max)(m_ModelBox.maxPoint.y, tmp.y);
		m_ModelBox.maxPoint.z = (std::max)(m_ModelBox.maxPoint.z, tmp.z);

		m_ModelBox.minPoint.x = (std::min)(m_ModelBox.minPoint.x, tmp2.x);
		m_ModelBox.minPoint.y = (std::min)(m_ModelBox.minPoint.y, tmp2.y);
		m_ModelBox.minPoint.z = (std::min)(m_ModelBox.minPoint.z, tmp2.z);

		m_ModelBox.maxPoint.x = (std::max)(m_ModelBox.maxPoint.x, tmp2.x);
		m_ModelBox.maxPoint.y = (std::max)(m_ModelBox.maxPoint.y, tmp2.y);
		m_ModelBox.maxPoint.z = (std::max)(m_ModelBox.maxPoint.z, tmp2.z);

	}
}

void FGLTFMode::UpdateModelMatrix()
{
	m_ModelMatrix = FMatrix::ScaleMatrix(m_Scale) * m_RotationMatrix;
}

