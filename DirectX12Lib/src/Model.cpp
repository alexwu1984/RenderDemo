#include "Model.h"
#include "ObjLoader.h"
#include "MeshData.h"
#include "CommandContext.h"
#include "Material.h"
#include "RenderPipelineInfo.h"
#include "AiObjLoader.h"

FModel::FModel(const std::wstring& FileName)
	: m_FileName(FileName)
	, m_Scale(1.f)
{
	m_UseAiMesh = true;
	if (m_UseAiMesh)
	{
		m_AiMeshDataWapper.m_MeshData = FAiObjLoader::GetLoader().LoadObj(FileName);
	}
	else
	{
		m_MeshDataWapper.m_MeshData = FObjLoader::LoadObj(FileName);
	}
	InitializeResource();
}

FModel::~FModel()
{
	m_MeshDataWapper.m_MeshData.reset();
	m_AiMeshDataWapper.m_MeshData.reset();
}

void FModel::Draw(FCommandContext& CommandContext, FRenderItem* renderItem)
{
	if (m_UseAiMesh)
	{
		const auto& SubMeshDataList = m_AiMeshDataWapper.m_MeshData->GetSubMeshData();
		for (int32_t index = SubMeshDataList.size() - 1 ; index >= 0; --index)
		{
			const auto& SubMeshData = SubMeshDataList[index];
			for (int vI = 0, slot = 0; vI < VET_Max; ++vI)
			{
				if (SubMeshData.HasVertexElement(VertexElementType(vI)))
				{
					CommandContext.SetVertexBuffer(slot++, SubMeshData.VertexBuffer[vI].VertexBufferView());
				}
			}
			CommandContext.SetIndexBuffer(SubMeshData.IndexBuffer.IndexBufferView());

			std::shared_ptr<FMaterial> Material = m_AiMeshDataWapper.m_Materials[SubMeshData.MaterialIndex];
			if (Material->GetTexture().GetResource())
			{
				CommandContext.SetDynamicDescriptor(m_textureIndex, 0, Material->GetTexture().GetSRV());				
			}

			if (renderItem)
			{
				CommandContext.SetDynamicDescriptor(renderItem->CBVRootIndex, 0, renderItem->MapBasePassInfos[SubMeshData.MaterialIndex]->BasePassCpuHandle);
			}


			if (!m_UseOutsideColor)
			{
				auto [kd, ks] = Material->GetColor();
				m_lightMaterial.uKd = kd;
				m_lightMaterial.uKs = ks;
			}
			memcpy(m_LightMaterialConstBuf.Map(), &m_lightMaterial, sizeof(m_lightMaterial));

			CommandContext.SetDynamicDescriptor(m_lightMaterialIndex, 0, m_LightMaterialCpuHandle);

			if (m_DrawParam)
			{
				m_DrawParam(CommandContext, Material->GetTexture().GetResource() ? 1 : 0);
			}

			CommandContext.DrawIndexed(SubMeshData.IndexCount, 0);
		}

	}
	else
	{
		for (int i = 0, slot = 0; i < VET_Max; ++i)
		{
			if (m_MeshDataWapper.m_MeshData->HasVertexElement(VertexElementType(i)))
			{
				CommandContext.SetVertexBuffer(slot++, m_MeshDataWapper.m_VertexBuffer[i].VertexBufferView());
			}
		}
		CommandContext.SetIndexBuffer(m_MeshDataWapper.m_IndexBuffer.IndexBufferView());

		for (size_t i = 0; i < m_MeshDataWapper.m_MeshData->GetMeshCount(); ++i)
		{
			size_t MtlIndex = m_MeshDataWapper.m_MeshData->GetSubMaterialIndex(i);
			std::shared_ptr<FMaterial> Material = m_MeshDataWapper.m_Materials[MtlIndex];

			if (Material->GetTexture().GetResource())
			{
				CommandContext.SetDynamicDescriptor(m_textureIndex, 0, Material->GetTexture().GetSRV());
			}

			if (renderItem)
			{
				CommandContext.SetDynamicDescriptor(renderItem->CBVRootIndex, 0, renderItem->MapBasePassInfos[MtlIndex]->BasePassCpuHandle);
			}

			if (!m_UseOutsideColor)
			{
				auto [kd, ks] = Material->GetColor();
				m_lightMaterial.uKd = kd;
				m_lightMaterial.uKs = ks;
			}
			memcpy(m_LightMaterialConstBuf.Map(), &m_lightMaterial, sizeof(m_lightMaterial));

			CommandContext.SetDynamicDescriptor(m_lightMaterialIndex, 0, m_LightMaterialCpuHandle);

			if (m_DrawParam)
			{
				m_DrawParam(CommandContext, Material->GetTexture().GetResource() ? 1 : 0);
			}

			CommandContext.DrawIndexed((UINT)m_MeshDataWapper.m_MeshData->GetSubIndexCount(i), (UINT)m_MeshDataWapper.m_MeshData->GetSubIndexStart(i));
		}
	}

}

void FModel::GetMeshLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& MeshLayout)
{
	MeshLayout.clear();

	if (m_UseAiMesh)
	{
		GetMeshLayout(m_AiMeshDataWapper.m_MeshData, MeshLayout);
	}
	else
	{
		GetMeshLayout(m_MeshDataWapper.m_MeshData, MeshLayout);
	}

}

void FModel::SetScale(float Scale)
{
	m_Scale = Scale;
	UpdateModelMatrix();
}

void FModel::SetRotation(const FMatrix& Rotation)
{
	m_RotationMatrix = Rotation;
	UpdateModelMatrix();
}

void FModel::SetPosition(const Vector3f& Position)
{
	m_Position = Position;
	UpdateModelMatrix();
}

void FModel::SetPosition(float x, float y, float z)
{
	m_Position = Vector3f(x, y, z);
	UpdateModelMatrix();
}

Vector3f FModel::GetPosition() const
{
	return m_Position;
}

void FModel::SetLightDir(const Vector3f& lightDir)
{
	m_lightMaterial.uLightDir = lightDir;
}

void FModel::SetLightColor(const Vector4f& lightColor)
{
	m_lightMaterial.uLightColor;
}

void FModel::SetLightIntensity(const Vector3f& LightIntensity)
{
	m_lightMaterial.uLightIntensity = LightIntensity;
}

void FModel::SetCameraPos(const Vector3f& cameraPos)
{
	m_lightMaterial.uCameraPos = cameraPos;
}

void FModel::SetLightMVP(FMatrix modelMatrix, FMatrix viewMatrix, FMatrix projectionMatrix)
{
	m_lightMaterial.modelMatrix = modelMatrix;
	m_lightMaterial.viewMatrix = viewMatrix;
	m_lightMaterial.projectionMatrix = projectionMatrix;
}

void FModel::UpdateModelMatrix()
{
	m_ModelMatrix = FMatrix::ScaleMatrix(m_Scale) * m_RotationMatrix * FMatrix::TranslateMatrix(m_Position);
}

void FModel::InitializeResource()
{
	if (m_UseAiMesh)
	{

		for (auto MatItem : m_AiMeshDataWapper.m_MeshData->GetAllMaterialData())
		{
			std::shared_ptr<FMaterial> material = std::make_shared<FMaterial>();
			auto MtlData = m_AiMeshDataWapper.m_MeshData->GetMaterialData(MatItem.first);
			if (!MtlData.DiffuseTexPath.empty())
			{
				material->LoadTexture(MtlData.DiffuseTexPath);
			}
			material->SetColor(MtlData.Kd, MtlData.Ks);
			m_AiMeshDataWapper.m_Materials.insert({ MatItem.first,material });
		}
	}
	else
	{
		for (int i = 0; i < VET_Max; ++i)
		{
			VertexElementType elmType = VertexElementType(i);
			if (m_MeshDataWapper.m_MeshData->HasVertexElement(elmType))
			{
				m_MeshDataWapper.m_VertexBuffer[i].Create(
					L"VertexStream",
					m_MeshDataWapper.m_MeshData->GetVertexCount(),
					m_MeshDataWapper.m_MeshData->GetVertexStride(elmType),
					m_MeshDataWapper.m_MeshData->GetVertexData(elmType));
			}
		}

		m_MeshDataWapper.m_IndexBuffer.Create(L"MeshIndexBuffer", m_MeshDataWapper.m_MeshData->GetIndexCount(), m_MeshDataWapper.m_MeshData->GetIndexElementSize(), m_MeshDataWapper.m_MeshData->GetIndexData());

		size_t MaterialCount = m_MeshDataWapper.m_MeshData->GetMaterialCount();
		m_MeshDataWapper.m_Materials.reserve(MaterialCount);

		for (size_t i = 0; i < MaterialCount; ++i)
		{
			std::shared_ptr<FMaterial> material = std::make_shared<FMaterial>();
			MaterialData MtlData = m_MeshDataWapper.m_MeshData->GetMaterialData(i);
			if (!MtlData.BaseColorPath.empty())
			{
				material->LoadTexture(MtlData.BaseColorPath);
			}
			material->SetColor(MtlData.Kd, MtlData.Ks);
			m_MeshDataWapper.m_Materials.push_back(material);
		}
	}


	m_LightMaterialConstBuf.CreateUpload(L"lightMaterial", sizeof(m_lightMaterial));
	m_LightMaterialCpuHandle = m_LightMaterialConstBuf.CreateConstantBufferView(0, sizeof(m_lightMaterial));
	m_lightMaterial.shadowtype = 0;
	m_lightMaterial.uLightColor = Vector4f(1.0);
}

 BoundingBox FModel::GetBoundBox() const
{
	if (!m_MeshDataWapper.m_MeshData)
	{
		return BoundingBox();
	}
	return m_MeshDataWapper.m_MeshData->GetBoundBox();
 }

 void FModel::SetDrawParam(const std::function<void(FCommandContext&, bool hasTexture)>& fun)
 {
	 m_DrawParam = fun;
 }

 void FModel::SetShadowType(int32_t shadowType)
 {
	 m_lightMaterial.shadowtype = shadowType;
 }

 void FModel::InitRootIndex(int lightMaterialIndex, int textureIndex)
 {
	 m_lightMaterialIndex = lightMaterialIndex;
	 m_textureIndex = textureIndex;
 }

 void FModel::SetColor(const Vector3f& color)
 {
	 m_lightMaterial.uKd = color;
	 m_UseOutsideColor = true;
	 memcpy(m_LightMaterialConstBuf.Map(), &m_lightMaterial, sizeof(m_lightMaterial));
 }

 bool FModel::HasTexture() const
 {
	 if (m_UseAiMesh)
	 {
		 for (size_t i = 0; i < m_AiMeshDataWapper.m_MeshData->GetMeshCount(); ++i)
		 {
			 size_t MtlIndex = m_AiMeshDataWapper.m_MeshData->GetSubMaterialIndex(i);
			 auto it = m_AiMeshDataWapper.m_Materials.find(MtlIndex);
			 if (it != m_AiMeshDataWapper.m_Materials.end())
			 {
				 std::shared_ptr<FMaterial> material = it->second;
				 if (material->GetTexture().GetResource())
				 {
					 return true;
				 }
			 }

		 }
	 }
	 else
	 {
		 for (size_t i = 0; i < m_MeshDataWapper.m_MeshData->GetMeshCount(); ++i)
		 {
			 size_t MtlIndex = m_MeshDataWapper.m_MeshData->GetSubMaterialIndex(i);
			 std::shared_ptr<FMaterial> material = m_MeshDataWapper.m_Materials[MtlIndex];

			 if (material->GetTexture().GetResource())
			 {
				 return true;
			 }
		 }
	 }

	 return false;
 }

 void FRenderItem::Init()
 {
	 std::shared_ptr< BasePassInfoWrapper> InfoWrapper = std::make_shared<BasePassInfoWrapper>();
	 InfoWrapper->BasePassInfo.mUseTex = 0;
	 InfoWrapper->BasePassConstBuf.CreateUpload(L"BasePassInfo", sizeof(InfoWrapper->BasePassInfo));
	 InfoWrapper->BasePassCpuHandle = InfoWrapper->BasePassConstBuf.CreateConstantBufferView(0, sizeof(InfoWrapper->BasePassInfo));
	 MapBasePassInfos.insert({ 0,InfoWrapper });
 }

 void FRenderItem::Init(const std::wstring& path)
 {
	 Model = std::make_shared<FModel>(path);
	 const auto& SubMeshDataList = Model->m_AiMeshDataWapper.m_MeshData->GetSubMeshData();
	 for (const auto& SubMeshData : SubMeshDataList)
	 {
		 std::shared_ptr<FMaterial> Material = Model->m_AiMeshDataWapper.m_Materials[SubMeshData.MaterialIndex];

		 std::shared_ptr< BasePassInfoWrapper> InfoWrapper = std::make_shared<BasePassInfoWrapper>();
		 InfoWrapper->BasePassInfo.mUseTex = Material->GetTexture().GetResource() ? 1 : 0;
		 InfoWrapper->BasePassConstBuf.CreateUpload(L"BasePassInfo", sizeof(InfoWrapper->BasePassInfo));
		 InfoWrapper->BasePassCpuHandle = InfoWrapper->BasePassConstBuf.CreateConstantBufferView(0, sizeof(InfoWrapper->BasePassInfo));
		 MapBasePassInfos.insert({ SubMeshData.MaterialIndex,InfoWrapper });
	 }
 }
