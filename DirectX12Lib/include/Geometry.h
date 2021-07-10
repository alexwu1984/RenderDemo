#pragma once
#include "Common.h"
#include "CommandContext.h"
#include "GpuBuffer.h"
#include "MathLib.h"

class FGeometry
{
public:
	struct Vertex
	{
		float position[3];
		float tex[2];
		float normal[3];
	};
public:
	FGeometry();
	~FGeometry();

	void CreateRectange();
	void CreateCube(float width = 100, float height = 100, float depth = 100);
	void CreateQuad(float x, float y, float w, float h, float depth);
	
	void SetVertex(const std::vector<Vertex> vertexBufferData, const std::vector<uint32_t> indexBufferData);
	void GetMeshLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& MeshLayout);
	void Draw(FCommandContext& CommandContext);

	void SetScale(float Scale);
	float GetScale();
	void SetRotation(const FMatrix& Rotation);
	void SetPosition(const Vector3f& Position);
	const FMatrix GetModelMatrix() { return m_ModelMatrix; }
	const BoundingBox& GetBoundBox() const { return m_boundingBox; }
private:
	void UpdateModelMatrix();
	void UpdateBoundingBox(const Vector3f& pos, Vector3f& vMin, Vector3f& vMax);
	

private:

	FGpuBuffer m_VertexBuffer;
	FGpuBuffer m_IndexBuffer;

	float m_Scale = 1.0;
	FMatrix m_RotationMatrix;
	Vector3f m_Position;
	FMatrix m_ModelMatrix;

	BoundingBox m_boundingBox;
};