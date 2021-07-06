#include "Geometry.h"

FGeometry::FGeometry()
{

}

FGeometry::~FGeometry()
{

}

void FGeometry::CreateRectange()
{
	std::vector<Vertex> vertexBufferData =
	{
		{ { -1.f,  1.f, -1.f }, {0.f, 0.f},{ 0,  0, 1.f } }, // 0
		{ {  1.f,  1.f, -1.f }, {1.f, 0.f},{ 0,  0, 1.f }  }, // 1
		{ {  1.f, -1.f, -1.f }, {1.f, 1.f},{ 0,  0, 1.f }  }, // 2
		{ { -1.f, -1.f, -1.f }, {0.f, 1.f},{ 0,  0, 1.f }  }, // 3
	};

	std::vector<uint32_t> indexBufferData= {
	0, 1, 2,
	0, 2, 3,
	};

	SetVertex(vertexBufferData, indexBufferData);
}

void FGeometry::CreateCube()
{
	std::vector<Vertex> v(24);
	float width = 100;
	float height = 100;
	float depth = 100;

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Fill in the front face vertex data.
	v[0] = Vertex({ { -w2, -h2, -d2 },{ 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } });
	v[1] = Vertex({ { -w2, +h2, -d2 },{ 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } });
	v[2] = Vertex({ { +w2, +h2, -d2 },{ 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } });
	v[3] = Vertex({ { +w2, -h2, -d2 },{ 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } });

	//// Fill in the back face vertex data.
	v[4] = Vertex({ { -w2, -h2, +d2 },{ 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } });
	v[5] = Vertex({ { +w2, -h2, +d2 },{ 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } });
	v[6] = Vertex({ { +w2, +h2, +d2},{ 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } });
	v[7] = Vertex({ { -w2, +h2, +d2 },{ 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } });

	//// Fill in the top face vertex data.
	v[8] = Vertex({ { -w2, +h2, -d2 },{ 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } });
	v[9] = Vertex({ { -w2, +h2, +d2 },{ 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } });
	v[10] = Vertex({ { +w2, +h2, +d2},{ 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } });
	v[11] = Vertex({ { +w2, +h2, -d2 },{ 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } });

	//// Fill in the bottom face vertex data.
	v[12] = Vertex({ { -w2, -h2, -d2 },{ 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } });
	v[13] = Vertex({ { +w2, -h2, -d2 },{ 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } });
	v[14] = Vertex({ { +w2, -h2, +d2 },{ 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } });
	v[15] = Vertex({ { -w2, -h2, +d2 },{ 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } });

	//// Fill in the left face vertex data.
	v[16] = Vertex({ { -w2, -h2, +d2 },{ 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } });
	v[17] = Vertex({ { -w2, +h2, +d2 },{ 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } });
	v[18] = Vertex({ { -w2, +h2, -d2 },{ 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } });
	v[19] = Vertex({ { -w2, -h2, -d2 },{ 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } });

	//// Fill in the right face vertex data.
	v[20] = Vertex({ { +w2, -h2, -d2 },{ 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } });
	v[21] = Vertex({ { +w2, +h2, -d2 },{ 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } });
	v[22] = Vertex({ { +w2, +h2, +d2 },{ 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } });
	v[23] = Vertex({ { +w2, -h2, +d2 },{ 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } });

	std::vector<uint32_t> i(36);

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	SetVertex(v, i);
}

void FGeometry::CreateQuad(float x, float y, float w, float h, float depth)
{
	std::vector<Vertex> Vertices(4);
	std::vector<uint32_t> Indices32(6);

	// Position coordinates specified in NDC space.
	Vertices[0] = Vertex(
		{ { x, y - h, depth },
		{ 0.0f, 1.0f },
		{ 0.0f, 0.0f, -1.0f },
		});

	Vertices[1] = Vertex(
		{ { x, y, depth },
		{ 0.0f, 0.0f },
		{ 0.0f, 0.0f, -1.0f },
		});

	Vertices[2] = Vertex(
		{ { x+w, y, depth },
		{ 1.0f, 0.0f },
		{ 0.0f, 0.0f, -1.0f },
		});

	Vertices[3] = Vertex(
		{ { x + w, y-h, depth },
		{ 1.0f, 1.0f },
		{ 0.0f, 0.0f, -1.0f },
		});


	Indices32[0] = 0;
	Indices32[1] = 1;
	Indices32[2] = 2;

	Indices32[3] = 0;
	Indices32[4] = 2;
	Indices32[5] = 3;

	SetVertex(Vertices, Indices32);
}

void FGeometry::SetVertex(const std::vector<Vertex> vertexBufferData, const std::vector<uint32_t> indexBufferData)
{
	m_VertexBuffer.Create(L"VertexBuffer", vertexBufferData.size(), sizeof(Vertex), vertexBufferData.data());
	m_IndexBuffer.Create(L"IndexBuffer", indexBufferData.size(), sizeof(uint32_t), indexBufferData.data());
}

void FGeometry::GetMeshLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& MeshLayout)
{
	MeshLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	MeshLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	MeshLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
}

void FGeometry::Draw(FCommandContext& CommandContext)
{
	CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandContext.SetVertexBuffer(0, m_VertexBuffer.VertexBufferView());
	CommandContext.SetIndexBuffer(m_IndexBuffer.IndexBufferView());

	CommandContext.DrawIndexed(m_IndexBuffer.GetElementCount());
}

void FGeometry::SetScale(float Scale)
{
	m_Scale = Scale;
	UpdateModelMatrix();
}

void FGeometry::SetRotation(const FMatrix& Rotation)
{
	m_RotationMatrix = Rotation;
	UpdateModelMatrix();
}

void FGeometry::SetPosition(const Vector3f& Position)
{
	m_Position = Position;
	UpdateModelMatrix();
}

void FGeometry::UpdateModelMatrix()
{
	m_ModelMatrix = FMatrix::ScaleMatrix(m_Scale) * m_RotationMatrix * FMatrix::TranslateMatrix(m_Position);
}
