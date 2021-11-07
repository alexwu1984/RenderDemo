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

