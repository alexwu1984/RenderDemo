#pragma once
#include "MeshData.h"
#include "GpuBuffer.h"

struct aiMesh;
struct aiScene;
struct aiMaterial;

struct FSubMeshData
{
	FSubMeshData();
	~FSubMeshData();

	bool HasVertexElement(VertexElementType type) const;
	uint32_t GetVertexCount() const;
	uint32_t GetVertexSize(VertexElementType type) const;
	uint32_t GetVertexStride(VertexElementType type) const;
	const float* GetVertexData(VertexElementType type);

	uint32_t GetIndexCount() const;
	uint32_t GetIndexSize() const;
	uint32_t GetIndexElementSize() const;
	const uint32_t* GetIndexData();

	uint32_t MaterialIndex = 0;
	uint32_t IndexCount;
	std::vector<Vector3f> Positions;
	std::vector<Vector3f> Colors;
	std::vector<Vector2f> Texcoords;
	std::vector<Vector3f> Normals;
	std::vector<Vector4f> Tangets;
	std::vector<uint32_t> Indices;

	FGpuBuffer VertexBuffer[VertexElementType::VET_Max];
	FGpuBuffer IndexBuffer;
	
};

struct FMaterialData
{
	std::string Name;
	std::wstring DiffuseTexPath;
	std::wstring RoughnessPath;
	std::wstring NormalTexPath;
	std::wstring EmissiveTexPath;
	std::wstring AmbientTexPath;
	std::wstring OpacityTexPath;
	std::wstring MetallicPath;
	//ambient color
	Vector3f Ka;
	// Diffuse Color
	Vector3f Kd;
	// Specular Color
	Vector3f Ks;

	float Shininess = 0.f;
	// Specular Exponent
	int32_t MaterialIndex = 0;
	
};

class FAiMeshData
{
public:
	FAiMeshData(const std::wstring& filepath);
	~FAiMeshData() {}

	bool HasVertexElement(VertexElementType type) const;

	size_t GetMeshCount() const { return m_Submeshes.size(); }
	size_t GetSubMaterialIndex(size_t Index) const;

	const FMaterialData& GetMaterialData(size_t Index);
	std::map<uint32_t, FMaterialData> GetAllMaterialData();
	void UpdateBoundingBox(const Vector3f& pos, Vector3f& vMin, Vector3f& vMax);
	const BoundingBoxDeprecated& GetBoundBox() const { return m_Bounds; }

	void TraverserNodes(const aiScene* scene);
	const std::vector<FSubMeshData>& GetSubMeshData() { return m_Submeshes; }
private:
	void ProcessVertex(const aiMesh* vAiMesh, FSubMeshData& meshData);
	uint32_t ProcessIndices(const aiMesh* vAiMesh, FSubMeshData& MeshData);
	void ProcessMaterialData(const aiScene* scene, const aiMesh* vAiMesh);
	void LoadTextureFromMaterial(int32_t vTextureType, const aiMaterial* vMat, std::wstring& texPath);

	void CalcTangents(
		const std::vector<Vector3f>& final_positions,
		const std::vector<Vector2f>& final_texcoords,
		const std::vector<Vector3f>& final_normals,
		const std::vector<uint32_t>& final_indices,
		std::vector<Vector4f>& final_tangents
	);

	friend FObjLoader;

protected:
	std::wstring m_Filepath;
	std::wstring m_Directory;

	std::map<uint32_t, FMaterialData> m_Materials;
	std::vector<FSubMeshData> m_Submeshes;
	BoundingBoxDeprecated m_Bounds;
};
