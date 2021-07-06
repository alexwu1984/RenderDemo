#include "AiMeshData.h"
#include "GpuBuffer.h"
#include "Texture.h"
#include "MathLib.h"
#include "Camera.h"
#include "Shader.h"
#include <string>


class FCommandContext;
class RenderPipelineInfo;
class FMaterial;
struct FRenderItem;

class FModel
{
	friend struct FRenderItem;
public:
	FModel(const std::wstring& FileName);
	~FModel();

	void Draw(FCommandContext& CommandContext, FRenderItem *renderItem = nullptr);

	void GetMeshLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& MeshLayout);

	void SetScale(float Scale);
	void SetRotation(const FMatrix& Rotation);
	void SetPosition(const Vector3f& Position);
	void SetPosition(float x, float y, float z);
	Vector3f GetPosition() const;
	const FMatrix GetModelMatrix() { return m_ModelMatrix; }
	uint32_t GetLightMaerialStride() { return sizeof(m_lightMaterial); }
	void SetLightDir(const Vector3f& lightDir);
	void SetLightColor(const Vector4f& lightColor);
	void SetLightIntensity(const Vector3f& LightIntensity);
	void SetCameraPos(const Vector3f& cameraPos);
	void SetLightMVP(FMatrix modelMatrix,FMatrix viewMatrix,FMatrix projectionMatrix);
	BoundingBox GetBoundBox() const;
	void SetDrawParam(const std::function<void(FCommandContext& , bool hasTexture)>& fun);
	void SetShadowType(int32_t shadowType);
	void InitRootIndex(int lightMaterialIndex, int textureIndex);
	void SetColor(const Vector3f& color);
	bool HasTexture() const;

private:
	void InitializeResource();
	void UpdateModelMatrix();
	template<typename MeshData> void GetMeshLayout(MeshData PtrData, std::vector<D3D12_INPUT_ELEMENT_DESC>& MeshLayout)
	{
		UINT slot = 0;
		if (PtrData->HasVertexElement(VET_Position))
		{
			MeshLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		}
		if (PtrData->HasVertexElement(VET_Color))
		{
			MeshLayout.push_back({ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		}
		if (PtrData->HasVertexElement(VET_Texcoord))
		{
			MeshLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		}
		if (PtrData->HasVertexElement(VET_Normal))
		{
			MeshLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot++, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		}
	}

protected:
	float m_Scale;
	FMatrix m_RotationMatrix;
	Vector3f m_Position;
	FMatrix m_ModelMatrix;

	std::wstring m_FileName;

	struct MeshDataWapper
	{
		std::shared_ptr<MeshData> m_MeshData;
		FGpuBuffer m_VertexBuffer[VET_Max];
		FGpuBuffer m_IndexBuffer;
		std::vector<std::shared_ptr< FMaterial>> m_Materials;
	};

	MeshDataWapper m_MeshDataWapper;

	struct AiMeshDataWapper
	{
		std::shared_ptr<FAiMeshData> m_MeshData;
		std::map<int32_t, std::shared_ptr< FMaterial>> m_Materials;
	};
	AiMeshDataWapper m_AiMeshDataWapper;

	bool m_UseAiMesh = true;

	struct
	{
		Vector3f uKd;
		float  pad1;
		Vector3f uKs;
		float  pad2;
		Vector3f uLightDir;
		float  pad4;
		Vector3f uCameraPos;
		float  pad5;
		Vector3f uLightIntensity;
		int32_t  shadowtype;// 0 none 1 pcf 2 vsm;
		Vector4f uLightColor;
		FMatrix projectionMatrix;
		FMatrix modelMatrix;
		FMatrix viewMatrix;
	}m_lightMaterial;

	FConstBuffer m_LightMaterialConstBuf;
	D3D12_CPU_DESCRIPTOR_HANDLE m_LightMaterialCpuHandle;
	std::function<void(FCommandContext&,bool hasTexture)> m_DrawParam;

	int32_t m_lightMaterialIndex = 1;
	int32_t m_textureIndex =2;
	bool m_UseOutsideColor = false;
};

struct FRenderItem
{
	FRenderItem() {}
	~FRenderItem() {}
	void Init();
	void Init(const std::wstring& path);


	struct BasePassInfoWrapper
	{
		struct
		{
			FMatrix projectionMatrix;
			FMatrix modelMatrix;
			FMatrix viewMatrix;
			int mUseTex = 0;
			Vector3i pad;
		} BasePassInfo;

		FConstBuffer BasePassConstBuf;
		D3D12_CPU_DESCRIPTOR_HANDLE BasePassCpuHandle;
	};

	std::map<int32_t, std::shared_ptr< BasePassInfoWrapper> > MapBasePassInfos;

	int CBVRootIndex = 0;

	std::shared_ptr<FModel> Model;
	std::shared_ptr<class FGeometry> Geo;
	std::shared_ptr<class RenderPipelineInfo> PiplelineInfo;
};