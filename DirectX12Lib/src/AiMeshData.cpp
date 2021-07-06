#include "AiMeshData.h"
#include "StringUnit.h"
#include <Assimp/Importer.hpp>
#include <Assimp/scene.h>
#include <Assimp/postprocess.h>

FSubMeshData::FSubMeshData()
{

}

FSubMeshData::~FSubMeshData()
{

}

bool FSubMeshData::HasVertexElement(VertexElementType type) const
{
	switch (type)
	{
	case VET_Position:
		return !Positions.empty();
	case VET_Color:
		return !Colors.empty();
	case VET_Texcoord:
		return !Texcoords.empty();
	case VET_Normal:
		return !Normals.empty();
	default:
		return nullptr;
	}
}

uint32_t FSubMeshData::GetVertexCount() const
{
	return Positions.size();
}

uint32_t FSubMeshData::GetVertexSize(VertexElementType type) const
{
	switch (type)
	{
	case VET_Position:
	case VET_Color:
	case VET_Normal:
		return (uint32_t)Positions.size() * sizeof(Vector3f);
	case VET_Texcoord:
		return (uint32_t)Texcoords.size() * sizeof(Vector2f);
	default:
		return 0;
	}
}

uint32_t FSubMeshData::GetVertexStride(VertexElementType type) const
{
	switch (type)
	{
	case VET_Position:
	case VET_Color:
	case VET_Normal:
		return sizeof(Vector3f);
	case VET_Texcoord:
		return sizeof(Vector2f);
	default:
		return 0;
	}
}

const float* FSubMeshData::GetVertexData(VertexElementType type)
{
	switch (type)
	{
	case VET_Position:
		return (float*)&Positions[0];
	case VET_Color:
		return (float*)&Colors[0];
	case VET_Texcoord:
		return (float*)&Texcoords[0];
	case VET_Normal:
		return (float*)&Normals[0];
	default:
		return nullptr;
	}
}

uint32_t FSubMeshData::GetIndexCount() const
{
	return (uint32_t)Indices.size();
}

uint32_t FSubMeshData::GetIndexSize() const
{
	return (uint32_t)Indices.size() * GetIndexElementSize();
}

uint32_t FSubMeshData::GetIndexElementSize() const
{
	return sizeof(uint32_t);
}

const uint32_t* FSubMeshData::GetIndexData()
{
	return &Indices[0];
}

FAiMeshData::FAiMeshData(const std::wstring& filepath)
{
	m_Filepath = filepath;
	m_Directory = m_Filepath.substr(0, m_Filepath.find_last_of('/'));
}

bool FAiMeshData::HasVertexElement(VertexElementType type) const
{
	for (auto &Item : m_Submeshes)
	{
		return Item.HasVertexElement(type);
	}
	return false;
}

size_t FAiMeshData::GetSubMaterialIndex(size_t Index) const
{
	auto itFind = m_Materials.find(Index);
	if (itFind != m_Materials.end())
	{
		return itFind->first;
	}
	return (size_t)-1;
}

const FMaterialData& FAiMeshData::GetMaterialData(size_t Index)
{
	return m_Materials[Index];
}

std::map<uint32_t, FMaterialData> FAiMeshData::GetAllMaterialData()
{
	return m_Materials;
}

void FAiMeshData::UpdateBoundingBox(const Vector3f& pos, Vector3f& vMin, Vector3f& vMax)
{
	vMin = Vector3Min(vMin, pos);
	vMax = Vector3Max(vMax, pos);
	m_Bounds.Center = 0.5f * (vMin + vMax);
	m_Bounds.Extents = 0.5f * (vMax - vMin);
}

void FAiMeshData::TraverserNodes(const aiScene* scene)
{
	for (int32_t i = 0; i < scene->mNumMeshes; ++i)
	{
		const aiMesh* vAiMesh = scene->mMeshes[i];
		FSubMeshData SubMeshData;
		SubMeshData.MaterialIndex = vAiMesh->mMaterialIndex;
		ProcessVertex(vAiMesh, SubMeshData);
		SubMeshData.IndexCount = ProcessIndices(vAiMesh, SubMeshData);
		ProcessMaterialData(scene, vAiMesh);

		aiMaterial* pAiMat = scene->mMaterials[SubMeshData.MaterialIndex];
		FMaterialData& MaterialData = m_Materials[SubMeshData.MaterialIndex];

		LoadTextureFromMaterial(aiTextureType_DIFFUSE, pAiMat, MaterialData.DiffuseTexPath);
		LoadTextureFromMaterial(aiTextureType_NORMALS, pAiMat, MaterialData.NormalTexPath);
		LoadTextureFromMaterial(aiTextureType_SPECULAR, pAiMat, MaterialData.SpecularTexPath);

		for (int i = 0; i < VET_Max; ++i)
		{
			VertexElementType elmType = VertexElementType(i);
			if (SubMeshData.HasVertexElement(elmType))
			{
				SubMeshData.VertexBuffer[i].Create(
					L"VertexStream",
					SubMeshData.GetVertexCount(),
					SubMeshData.GetVertexStride(elmType),
					SubMeshData.GetVertexData(elmType));
			}
		}

		SubMeshData.IndexBuffer.Create(L"MeshIndexBuffer", SubMeshData.GetIndexCount(), SubMeshData.GetIndexElementSize(), SubMeshData.GetIndexData());

		m_Submeshes.emplace_back(SubMeshData);
	}
	
}

void FAiMeshData::ProcessVertex(const aiMesh* vAiMesh, FSubMeshData& meshData)
{
	Assert(vAiMesh);
	int32_t NumVertices = vAiMesh->mNumVertices;
	for (int32_t i = 0; i < NumVertices; ++i)
	{
		meshData.Positions.push_back({ vAiMesh->mVertices[i].x, vAiMesh->mVertices[i].y, vAiMesh->mVertices[i].z });
		if (vAiMesh->mNormals != nullptr)
		{
			meshData.Normals.push_back({ vAiMesh->mNormals[i].x, vAiMesh->mNormals[i].y, vAiMesh->mNormals[i].z });
		}

		if (vAiMesh->mTextureCoords[0])
		{
			meshData.Texcoords.push_back({ vAiMesh->mTextureCoords[0][i].x, vAiMesh->mTextureCoords[0][i].y });
		}

		/*if (vAiMesh->mTangents)*/
	}
}

uint32_t FAiMeshData::ProcessIndices(const aiMesh* vAiMesh, FSubMeshData& MeshData)
{
	int32_t NumFaces = vAiMesh->mNumFaces;
	uint32_t IndexCount = 0;
	for (int32_t i = 0; i < NumFaces; ++i)
	{
		aiFace AiFace = vAiMesh->mFaces[i];
		int32_t NumIndices = AiFace.mNumIndices;
		IndexCount += NumIndices;
		for (int32_t k = 0; k < NumIndices; ++k)
			MeshData.Indices.push_back(AiFace.mIndices[k]);
	}
	return IndexCount;
}

void FAiMeshData::ProcessMaterialData(const aiScene* scene, const aiMesh* vAiMesh)
{
	if (vAiMesh->mMaterialIndex < 0)
		return;
	aiMaterial* pAiMat = scene->mMaterials[vAiMesh->mMaterialIndex];
	aiColor3D AmbientColor, DiffuseColor, SpecularColor;
	float Shininess = 0.0f, Refracti = 0.0f;
	pAiMat->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor);
	pAiMat->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor);
	pAiMat->Get(AI_MATKEY_COLOR_SPECULAR, SpecularColor);
	pAiMat->Get(AI_MATKEY_SHININESS, Shininess);
	pAiMat->Get(AI_MATKEY_REFRACTI, Refracti);

	FMaterialData MaterialData;
	MaterialData.Ka = { AmbientColor.r, AmbientColor.g, AmbientColor.b };
	MaterialData.Kd = { DiffuseColor.r, DiffuseColor.g, DiffuseColor.b };
	MaterialData.Ks = { SpecularColor.r, SpecularColor.g, SpecularColor.b };
	MaterialData.Shininess = Shininess;
	
	m_Materials.insert({vAiMesh->mMaterialIndex,MaterialData});
}

void FAiMeshData::LoadTextureFromMaterial(int32_t vTextureType, const aiMaterial* vMat, std::wstring& texPath)
{
	int32_t TextureCount = vMat->GetTextureCount(static_cast<aiTextureType>(vTextureType));
	for (int32_t i = 0; i < TextureCount; ++i)
	{
		aiString Str;
		vMat->GetTexture(static_cast<aiTextureType>(vTextureType), i, &Str);
		std::wstring TextureName = core::u8_ucs2(Str.C_Str(),Str.length);
		texPath = m_Directory + L"/" + TextureName;
	}

}
