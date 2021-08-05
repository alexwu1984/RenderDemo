#pragma once

#include "MathLib.h"
#include <vector>
#include <string>
#include <optional>

enum VertexElementType : uint8_t
{
	VET_Position	= 0,
	VET_Color		= 1,
	VET_Texcoord	= 2,
	VET_Normal		= 3,
	VET_Tangent     = 4,
	VET_Max			= 5
};


struct VertexElement
{
	std::string SemanticName;
	uint32_t SemanticIndex;
	uint32_t Format;
	uint32_t Slot;
	uint32_t Size;
};


struct MaterialData
{
	std::string Name;
	std::wstring BaseColorPath;
	std::wstring NormalPath;
	std::wstring MetallicPath;
	std::wstring RoughnessPath;
	std::wstring AoPath;
	std::wstring OpacityPath;
	std::wstring EmissivePath;
	//ambient color
	Vector3f Ka;
	// Diffuse Color
	Vector3f Kd;
	// Specular Color
	Vector3f Ks;
	// Specular Exponent
	float Metallic = 0.f;
	float Roughness = 0.f;
};

struct SubMeshData
{
	SubMeshData(uint32_t InStartIndex, uint32_t InIndexCount, uint32_t InMaterialIndex)
		: StartIndex(InStartIndex)
		, IndexCount(InIndexCount)
		, MaterialIndex(InMaterialIndex)
	{}

	uint32_t MaterialIndex;
	uint32_t StartIndex, IndexCount;
};

class FObjLoader;
class FSkyBox;
class FCubeMapCross;

class MeshData
{
public:
	MeshData(const std::wstring& filepath);
	~MeshData();

	bool HasVertexElement(VertexElementType type) const;

	uint32_t GetVertexCount() const;
	uint32_t GetVertexSize(VertexElementType type) const;
	uint32_t GetVertexStride(VertexElementType type) const;
	const float* GetVertexData(VertexElementType type);

	uint32_t GetIndexCount() const;
	uint32_t GetIndexSize() const;
	uint32_t GetIndexElementSize() const;
	const uint32_t* GetIndexData();

	size_t GetMaterialCount() const { return m_materials.size(); }
	size_t GetMeshCount() const { return m_submeshes.size(); }
	uint32_t GetSubIndexStart(size_t Index) const;
	size_t GetSubIndexCount(size_t Index) const;
	size_t GetSubMaterialIndex(size_t Index) const;

	void AddMaterial(const MaterialData& Material);
	void AddSubMesh(uint32_t StartIndex, uint32_t IndexCount, uint32_t MaterialIndex);
	std::wstring GetBaseColorPath(uint32_t Index);
	const MaterialData& GetMaterialData(size_t Index);
	void UpdateBoundingBox(const Vector3f& pos, Vector3f& vMin, Vector3f& vMax);
	const BoundingBox& GetBoundBox() const { return m_bounds; }

	void ComputeBoundingBox();

	friend FObjLoader;
	friend FSkyBox;
	friend FCubeMapCross;

protected:
	std::wstring m_filepath;
	std::vector<Vector3f> m_positions;
	std::vector<Vector3f> m_colors;
	std::vector<Vector2f> m_texcoords;
	std::vector<Vector3f> m_normals;
	std::vector<Vector4f> m_tangents;
	std::vector<uint32_t> m_indices;

	std::vector<MaterialData> m_materials;
	std::vector<SubMeshData> m_submeshes;
	BoundingBox m_bounds;
};


class MeshPlane : public MeshData
{
public:
	MeshPlane();
};

struct FVertex
{
	// Position Vector
	Vector3f Position;

	// Normal Vector
	Vector3f Normal;

	// Texture Coordinate Vector
	Vector2f TextureCoordinate;
};

//struct FMaterialInfo
//{
//	FMaterialInfo()
//	{
//		name = "";
//		Ns = 0.0f;
//		Ni = 0.0f;
//		d = 0.0f;
//		illum = 0;
//	}
//
//	// Material Name
//	std::string name;
//	// Ambient Color
//	Vector3f Ka;
//	// Diffuse Color
//	Vector3f Kd;
//	// Specular Color
//	Vector3f Ks;
//	// Specular Exponent
//	float Ns;
//	// Optical Density
//	float Ni;
//	// Dissolve
//	float d;
//	// Illumination
//	int illum;
//	// Ambient Texture Map
//	std::string map_Ka;
//	// Diffuse Texture Map
//	std::string map_Kd;
//	// Specular Texture Map
//	std::string map_Ks;
//	// Specular Hightlight Map
//	std::string map_Ns;
//	// Alpha Texture Map
//	std::string map_d;
//	// Bump Map
//	std::string map_bump;
//};
//
//// Structure: Mesh
////
//// Description: A Simple Mesh Object that holds
////	a name, a vertex list, and an index list
//struct FMesh
//{
//	// Default Constructor
//	FMesh()
//	{
//
//	}
//	// Variable Set Constructor
//	FMesh(std::vector<FVertex>& _Vertices, std::vector<unsigned int>& _Indices)
//	{
//		Vertices = _Vertices;
//		Indices = _Indices;
//		MeshMaterial = std::nullopt;
//	}
//	// Mesh Name
//	std::string MeshName;
//	// Vertex List
//	std::vector<FVertex> Vertices;
//	// Position Vector
//
//	std::vector<Vector3f> Position;
//	// Normal Vector
//	std::vector <Vector3f> Normal;
//	// Texture Coordinate Vector
//	std::vector<Vector2f> TextureCoordinate;
//
//	// Index List
//	std::vector<unsigned int> Indices;
//
//	// Material
//	std::optional<FMaterialInfo> MeshMaterial;
//};
//
//struct FMeshData
//{
//	// Loaded Mesh Objects
//	std::vector<FMesh> LoadedMeshes;
//
//};
