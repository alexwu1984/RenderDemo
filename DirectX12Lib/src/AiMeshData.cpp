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
	case VET_Tangent:
		return !Tangets.empty();
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
	case VET_Tangent:
		return (uint32_t)Positions.size() * sizeof(Vector4f);
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
	case VET_Tangent:
		return sizeof(Vector4f);
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
	case VET_Tangent:
		return (float*)&Tangets[0];
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

		if (!SubMeshData.HasVertexElement(VET_Tangent))
		{
			CalcTangents(SubMeshData.Positions, SubMeshData.Texcoords, SubMeshData.Normals, SubMeshData.Indices, SubMeshData.Tangets);
		}

		//test
		//for (int32_t typeIndex = aiTextureType_DIFFUSE; typeIndex < aiTextureType_UNKNOWN; ++typeIndex)
		//{
		//	aiTextureType type = (aiTextureType)typeIndex;
		//	int32_t TextureCount = pAiMat->GetTextureCount(type);
		//	for (int32_t i = 0; i < TextureCount; ++i)
		//	{
		//		aiString Str;
		//		pAiMat->GetTexture(type, i, &Str);
		//		std::wstring TextureName = core::u8_ucs2(Str.C_Str(), Str.length);
		//		TextureName = m_Directory + L"/" + TextureName;
		//	}

		//}

		LoadTextureFromMaterial(aiTextureType_DIFFUSE, pAiMat, MaterialData.DiffuseTexPath);
		LoadTextureFromMaterial(aiTextureType_HEIGHT, pAiMat, MaterialData.NormalTexPath);
		LoadTextureFromMaterial(aiTextureType_SHININESS, pAiMat, MaterialData.RoughnessPath);
		LoadTextureFromMaterial(aiTextureType_EMISSIVE, pAiMat, MaterialData.EmissiveTexPath);
		LoadTextureFromMaterial(aiTextureType_AMBIENT, pAiMat, MaterialData.AmbientTexPath);
		LoadTextureFromMaterial(aiTextureType_OPACITY, pAiMat, MaterialData.OpacityTexPath);

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

		if (vAiMesh->mTangents)
		{
			meshData.Tangets.push_back({ vAiMesh->mTangents[i].x, vAiMesh->mTangents[i].y, vAiMesh->mTangents[i].z,0 });
		}
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

void FAiMeshData::CalcTangents(const std::vector<Vector3f>& final_positions, const std::vector<Vector2f>& final_texcoords, 
	                          const std::vector<Vector3f>& final_normals, const std::vector<uint32_t>& final_indices, 
	                          std::vector<Vector4f>& final_tangents)
{
	//Assert(ENABLE_TANGENT);
	Assert(final_indices.size() % 3 == 0);
	uint32_t TriangleCount = (uint32_t)final_indices.size() / 3;
	uint32_t VertexCount = (uint32_t)final_positions.size();

	std::vector<Vector3f> tan1;
	tan1.resize(VertexCount);
	std::vector<Vector3f> tan2;
	tan2.resize(VertexCount);
	for (uint32_t a = 0; a < TriangleCount; ++a)
	{
		uint32_t i1 = final_indices[3 * a];
		uint32_t i2 = final_indices[3 * a + 1];
		uint32_t i3 = final_indices[3 * a + 2];

		const Vector3f& v1 = final_positions[i1];
		const Vector3f& v2 = final_positions[i2];
		const Vector3f& v3 = final_positions[i3];
		const Vector2f& w1 = final_texcoords[i1];
		const Vector2f& w2 = final_texcoords[i2];
		const Vector2f& w3 = final_texcoords[i3];

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;
		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float div = s1 * t2 - s2 * t1;
		float r = div == 0.f ? 0.f : 1.f / div;

		Vector3f sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		Vector3f tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;
		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}
	for (uint32_t a = 0; a < VertexCount; a++)
	{
		// position(3), tex(2), normal(3), tangent(3)
		const Vector3f& n = final_normals[a];
		const Vector3f& t = tan1[a];
		// Gram-Schmidt orthogonalize.
		Vector4f tangent;
		tangent = (t - n * n.Dot(t)).SafeNormalize();
		// Calculate handedness.
		tangent.w = (Cross(n, t).Dot(tan2[a]) < 0.0f) ? -1.0f : 1.0f;
		final_tangents.push_back(tangent);
		if (isnan(tangent.x) || isnan(tangent.y) || isnan(tangent.z))
		{
			std::cout << "degenerated tangent, vertex index: %d" << a << std::endl;
		}
	}
}
