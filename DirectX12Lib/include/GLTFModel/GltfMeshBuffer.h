#pragma once
#include "Inc.h"
#include "GpuBuffer.h"
#include "MathLib.h"

struct FGltfMeshInfo;

__declspec(align(16)) struct FGltfVSConstants
{
	FMatrix ModelMatrix;
	FMatrix ViewProjMatrix;
	FMatrix PreviousModelMatrix;
	FMatrix PreviousViewProjMatrix;
	Vector2f ViewportSize;
};

class GltfMeshBuffer
{
public:
	enum VertexType : uint8_t
	{
		VT_Position = 0,
		VT_Texcoord = 1,
		VT_Normal = 2,
		VT_Tangent = 3,
		VT_Max = 4
	};
public:
	GltfMeshBuffer();
	~GltfMeshBuffer();

	void InitMesh(std::shared_ptr< FGltfMeshInfo> MeshInfo);
	void UpdateVert(Vector3f* pVert, int nVert);

	void GetMeshLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& MeshLayout);
	FGltfVSConstants& GetVS() { return m_VS; }
public:
	std::array<std::shared_ptr<FGpuBuffer>, VT_Max> VerticeBuffer;
	std::shared_ptr<FGpuBuffer> IndexBuffer;

	FGltfVSConstants m_VS;

	int AtrributeCount = 3;
};