#include "GltfMeshBuffer.h"
#include "GltfMesh.h"
#include "CommandContext.h"

GltfMeshBuffer::GltfMeshBuffer()
{

}

GltfMeshBuffer::~GltfMeshBuffer()
{

}

void GltfMeshBuffer::InitMesh(std::shared_ptr< FGltfMeshInfo> MeshInfo)
{
	VerticeBuffer[VT_Position] = std::make_shared<FGpuBuffer>();
	VerticeBuffer[VT_Position]->Create(L"Vertices", MeshInfo->nNumVertices, sizeof(Vector3f), MeshInfo->pVertices);

	VerticeBuffer[VT_Normal] = std::make_shared<FGpuBuffer>();
	VerticeBuffer[VT_Normal]->Create(L"Normals", MeshInfo->nNumVertices, sizeof(Vector3f), MeshInfo->pNormals);

	VerticeBuffer[VT_Texcoord] = std::make_shared<FGpuBuffer>();
	VerticeBuffer[VT_Texcoord]->Create(L"Texcoords", MeshInfo->nNumVertices, sizeof(Vector2f), MeshInfo->pTextureCoords);

	VerticeBuffer[VT_Tangent] = std::make_shared<FGpuBuffer>();
	VerticeBuffer[VT_Tangent]->Create(L"Tangents", MeshInfo->nNumVertices, sizeof(Vector4f), MeshInfo->pTangents);
}

void GltfMeshBuffer::UpdateVert(Vector3f* pVert, int nVert)
{
	if (VerticeBuffer[VT_Position])
	{
		FCommandContext::InitializeBuffer(*VerticeBuffer[VT_Position], pVert, nVert * sizeof(Vector3f));
	}
}

void GltfMeshBuffer::GetMeshLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& MeshLayout)
{
	uint32_t slot = 0;
	if (VerticeBuffer[VT_Position])
	{
		MeshLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	}

	if (VerticeBuffer[VT_Texcoord])
	{
		MeshLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	}

	if (VerticeBuffer[VT_Normal])
	{
		MeshLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	}

	if (VerticeBuffer[VT_Tangent])
	{
		MeshLayout.push_back({ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	}
}

