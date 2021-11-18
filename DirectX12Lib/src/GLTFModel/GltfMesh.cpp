#include "GltfMesh.h"
#include "GltfMaterial.h"
#include "GltfNode.h"
#include "GltfMeshBuffer.h"

FGltfMesh::FGltfMesh(tinygltf::Model* Model)
	:m_Model(Model)
{

}

FGltfMesh::~FGltfMesh()
{

}

void FGltfMesh::Init(uint32_t MeshIndex, uint32_t PrimitiveIndex, const std::vector < std::shared_ptr<FGltfMaterial>>& ModelMatrial, std::shared_ptr< FGltfNode> ModelNode)
{
	if (!m_Mesh)
	{
		m_Mesh = std::make_shared<FGltfMeshInfo>();
		m_GPUBuffer = std::make_shared<GltfMeshBuffer>();
	}

	auto& meshPrimitive = m_Model->meshes[MeshIndex].primitives[PrimitiveIndex];
	m_MeshName = m_Model->meshes[MeshIndex].name;

	auto Index = Getdata(meshPrimitive.indices, m_Mesh->nNumFaces, m_Mesh->type);
	m_Mesh->nNumFaces /= 3;
	if (m_Mesh->type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
	{
		m_Mesh->pFacesIndex = (uint16_t*)Index;
	}
	else if (m_Mesh->type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
	{
		//uint16_t* pData = new uint16_t[m_Mesh->nNumFaces * 3];

		std::shared_ptr<uint16_t> Data(new uint16_t[m_Mesh->nNumFaces * 3], [](uint16_t* p) {delete[]p; });
		uint8_t* pSrc = (uint8_t*)Index;
		for (uint32_t i = 0; i < m_Mesh->nNumFaces * 3; ++i)
		{
			Data.get()[i] = pSrc[i];
		}
		m_Mesh->pFacesIndex = Data.get();
		m_pData.push_back(Data);
	}
	else
	{
		m_Mesh->pFacesIndex32 = (uint32_t*)Index;
	}

	for (const auto& attribute : meshPrimitive.attributes) {

		int type = 0;
		if (attribute.first == "POSITION")
		{
			m_Mesh->pVertices = (Vector3f*)Getdata(attribute.second, m_Mesh->nNumVertices, type);
			auto& minVaue = m_Model->accessors[attribute.second].minValues;
			auto& maxVaue = m_Model->accessors[attribute.second].maxValues;
			if (minVaue.size() == 3 && maxVaue.size() == 3)
			{
				m_MeshBox.minPoint.x = float(minVaue[0]);
				m_MeshBox.minPoint.y = float(minVaue[1]);
				m_MeshBox.minPoint.z = float(minVaue[2]);
				m_MeshBox.maxPoint.x = float(maxVaue[0]);
				m_MeshBox.maxPoint.y = float(maxVaue[1]);
				m_MeshBox.maxPoint.z = float(maxVaue[2]);
				m_MeshBox.centerPoint = m_MeshBox.minPoint * 0.5f + m_MeshBox.maxPoint * 0.5f;
			}

		}
		else if (attribute.first == "NORMAL")
		{
			m_Mesh->pNormals = (Vector3f*)Getdata(attribute.second, m_Mesh->nNumVertices, type);
		}
		else if (attribute.first == "TEXCOORD_0")
		{
			m_Mesh->pTextureCoords = (Vector2f*)Getdata(attribute.second, m_Mesh->nNumVertices, type);
		}
		else if (attribute.first == "TANGENT")
		{
			m_Mesh->pTangents = (Vector4f*)Getdata(attribute.second, m_Mesh->nNumVertices, type);
		}
		//else if (attribute.first == "JOINTS_0")
		//{
		//	m_mesh->pBoneIDs = (CC3DVertexBoneID*)Getdata(attribute.second, m_mesh->nNumVertices, type);
		//}
		//else if (attribute.first == "WEIGHTS_0")
		//{
		//	m_mesh->pBoneWeights = (CC3DVertexBoneWeight*)Getdata(attribute.second, m_mesh->nNumVertices, type);
		//}
	}

	int nMaterial = meshPrimitive.material >= 0 ? meshPrimitive.material : 0;
	m_Material = ModelMatrial[nMaterial];
	if (m_Material->GetAlphaMode() != "OPAQUE")
	{
		m_isTransparent = true;
	}

	auto& Nodes = m_Model->nodes;
	for (int i = 0; i < Nodes.size(); i++)
	{
		if (Nodes[i].mesh == MeshIndex)
		{
			m_nNodeID = i;
			m_nSkinID = Nodes[i].skin;

			auto& AllNodeInfos = ModelNode->GetAllNodes();
			if (m_nNodeID < AllNodeInfos.size())
			{
				m_MeshMat = AllNodeInfos[m_nNodeID]->FinalMeshMat;
			}
			
			break;
		}

	}

	m_GPUBuffer->InitMesh(m_Mesh);
}

bool FGltfMesh::IsTransparent() const
{
	return m_isTransparent;
}

const FMatrix& FGltfMesh::GetMeshMat() const
{
	return m_MeshMat;
}

const FBoundingBox& FGltfMesh::GetBoundingBox() const
{
	return m_MeshBox;
}

std::shared_ptr<GltfMeshBuffer> FGltfMesh::GetGPUBuffer() const
{
	return m_GPUBuffer;
}

std::shared_ptr<FGltfMaterial> FGltfMesh::GetMaterial() const
{
	return m_Material;
}

void FGltfMesh::Update()
{
	m_PreviousModelMatrix = m_MeshMat;
}

FMatrix FGltfMesh::GetPreviousModelMatrix()
{
	return m_PreviousModelMatrix;
}

void* FGltfMesh::Getdata(int32_t attributeIndex, uint32_t& nCount, int32_t& CommpontType)
{
	const auto& indicesAccessor = m_Model->accessors[attributeIndex];
	const auto& bufferView = m_Model->bufferViews[indicesAccessor.bufferView];
	const auto& buffer = m_Model->buffers[bufferView.buffer];
	const auto dataAddress = buffer.data.data() + bufferView.byteOffset +
		indicesAccessor.byteOffset;
	const auto byteStride = indicesAccessor.ByteStride(bufferView);
	nCount = uint32_t(indicesAccessor.count);
	CommpontType = indicesAccessor.componentType;


	int type = indicesAccessor.type;
	int nStep = 0;
	if (type == TINYGLTF_TYPE_SCALAR) {
		nStep = 1;
	}
	else if (type == TINYGLTF_TYPE_VEC2) {
		nStep = 2;
	}
	else if (type == TINYGLTF_TYPE_VEC3) {

		nStep = 3;
	}
	else if (type == TINYGLTF_TYPE_VEC4) {

		nStep = 4;
	}
	int OneSize = 0;

	if (CommpontType == 5122 || CommpontType == 5123) {
		OneSize = sizeof(uint16_t);
	}
	else if (CommpontType == 5124 || CommpontType == 5125) {
		OneSize = sizeof(uint32_t);
	}
	else if (CommpontType == 5126) {

		OneSize = sizeof(float);
	}
	else if (CommpontType == 5120 || CommpontType == 5121) {
		OneSize = sizeof(uint8_t);
	}

	if (nStep == 0 || OneSize == 0 || nStep * OneSize == byteStride)
	{
		return (void*)dataAddress;
	}
	else
	{
		std::shared_ptr<uint8_t> Data(new uint8_t[nStep * OneSize * nCount], [](uint8_t* p) {delete[]p; });

		uint8_t* pSrc = (uint8_t*)dataAddress;
		for (uint32_t i = 0; i < nCount; ++i)
		{

			memcpy(Data.get() + i * OneSize * nStep, pSrc + i * byteStride, OneSize * nStep);
		}
		m_pData.push_back(Data);

		return (void*)Data.get();
	}
}

