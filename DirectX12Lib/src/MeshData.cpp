#include "MeshData.h"
#include "Common.h"

MeshData::MeshData(const std::wstring& filepath)
	: m_filepath(filepath)
{
}


MeshData::~MeshData()
{
}

bool MeshData::HasVertexElement(VertexElementType type) const
{
	switch (type)
	{
	case VET_Position:
		return !m_positions.empty();
	case VET_Color:
		return !m_colors.empty();
	case VET_Texcoord:
		return !m_texcoords.empty();
	case VET_Normal:
		return !m_normals.empty();
	case VET_Tangent:
		return !m_tangents.empty();
	default:
		return nullptr;
	}
}

uint32_t MeshData::GetVertexCount() const
{
	return (uint32_t)m_positions.size();
}

uint32_t MeshData::GetVertexSize(VertexElementType type) const
{
	switch (type)
	{
	case VET_Position:
	case VET_Color:
	case VET_Normal:
		return (uint32_t)m_positions.size() * sizeof(Vector3f);
	case VET_Texcoord:
		return (uint32_t)m_texcoords.size() * sizeof(Vector2f);
	case VET_Tangent:
		return (uint32_t)m_tangents.size() * sizeof(Vector4f);
	default:
		return 0;
	}
}

uint32_t MeshData::GetVertexStride(VertexElementType type) const
{
	switch (type)
	{
	case VET_Position:
	case VET_Color:
	case VET_Normal:
		return sizeof(Vector3f);
	case VET_Texcoord:
		return sizeof(Vector2f);
	case VET_Tangent:
		return sizeof(Vector4f);
	default:
		return 0;
	}
}

const float* MeshData::GetVertexData(VertexElementType type)
{
	switch (type)
	{
	case VET_Position:
		return (float*)&m_positions[0];
	case VET_Color:
		return (float*)&m_colors[0];
	case VET_Texcoord:
		return (float*)&m_texcoords[0];
	case VET_Normal:
		return (float*)&m_normals[0];
	case VET_Tangent:
		return (float*)&m_tangents[0];
	default:
		return nullptr;
	}
}

uint32_t MeshData::GetIndexSize() const
{
	return (uint32_t)m_indices.size() * GetIndexElementSize();
}

uint32_t MeshData::GetIndexElementSize() const
{
	//TODO: maybe short?
	return sizeof(uint32_t);
}

uint32_t MeshData::GetIndexCount() const
{
	return (uint32_t)m_indices.size();
}

const uint32_t* MeshData::GetIndexData()
{
	return &m_indices[0];
}

uint32_t MeshData::GetSubIndexStart(size_t Index) const
{
	Assert(Index < m_submeshes.size());
	return m_submeshes[Index].StartIndex;
}

size_t MeshData::GetSubIndexCount(size_t Index) const
{
	Assert(Index < m_submeshes.size());
	return m_submeshes[Index].IndexCount;
}

size_t MeshData::GetSubMaterialIndex(size_t Index) const
{
	Assert(Index < m_submeshes.size());
	return m_submeshes[Index].MaterialIndex;
}

void MeshData::AddMaterial(const MaterialData& Material)
{
	m_materials.push_back(Material);
}

void MeshData::AddSubMesh(uint32_t StartIndex, uint32_t IndexCount, uint32_t MaterialIndex)
{
	Assert(StartIndex < m_indices.size() && IndexCount <= m_indices.size());
	if (IndexCount == 0)
		IndexCount = (uint32_t)m_indices.size();
	m_submeshes.emplace_back(StartIndex, IndexCount, MaterialIndex);
}

std::wstring MeshData::GetBaseColorPath(uint32_t Index)
{
	std::wstring result;
	if (Index < m_submeshes.size())
	{
		uint32_t MtlIndex = m_submeshes[Index].MaterialIndex;
		if (MtlIndex < m_materials.size())
		{
			result = m_materials[MtlIndex].BaseColorPath;
		}
	}
	return result;
}

const MaterialData& MeshData::GetMaterialData(size_t Index)
{
	Assert(Index < m_materials.size());
	return m_materials[Index];
}

void MeshData::UpdateBoundingBox(const Vector3f& pos, Vector3f& vMin, Vector3f& vMax)
{
	vMin = Vector3Min(vMin, pos);
	vMax = Vector3Max(vMax, pos);
	m_bounds.Center = 0.5f * (vMin + vMax);
	m_bounds.Extents = 0.5f * (vMax - vMin);
}


void MeshData::ComputeBoundingBox()
{
	Vector3f BoundMin = Vector3f(std::numeric_limits<float>::max());
	Vector3f BoundMax = Vector3f(-std::numeric_limits<float>::max());
	for (size_t i = 0; i < m_positions.size(); ++i)
	{
		BoundMin = Vector3Min(BoundMin, m_positions[i]);
		BoundMax = Vector3Max(BoundMax, m_positions[i]);
	}
	m_bounds.Center = 0.5f * (BoundMin + BoundMax);
	m_bounds.Extents = 0.5f * (BoundMax - BoundMin);
}

MeshPlane::MeshPlane()
	: MeshData(L"Generated Plane")
{
	/*
		0-------1
		|		|
		|		|
		3-------2
	*/
	m_positions = {
		{ -1.f,  1.f, -1.f },
		{  1.f,  1.f, -1.f },
		{  1.f, -1.f, -1.f },
		{ -1.f, -1.f, -1.f },
	};
	//m_colors = {
	//	{ 1.f, 0.f, 0.f },
	//	{ 0.f, 1.f, 0.f },
	//	{ 0.f, 0.f, 1.f },
	//	{ 1.f, 1.f, 0.f },
	//};
	m_texcoords = {
		{0.f, 0.f},
		{1.f, 0.f},
		{1.f, 1.f},
		{0.f, 1.f},
	};

	m_indices = { 
		0, 1, 2,
		0, 2, 3,
	};
}
