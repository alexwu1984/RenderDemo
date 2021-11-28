#pragma once
#include "Inc.h"
#include "MathLib.h"
#include "tiny_gltf.h"

class FGltfMaterial;
class FGltfNode;
class GltfMeshBuffer;
/****************************************************
*	Mesh结构体
*	每个Mesh包含：
*		1、顶点数目
*		2、顶点位置
*		3、顶点颜色
*		4、顶点法向量
*		5、顶点纹理采样坐标
*		6、Faces数量
*		7、Faces索引
*		8、BoneIDs 影响该顶点的骨骼ID
*		9、BoneWeights 影响该顶点的骨骼权重
*		10、mesh使用的材质索引
*		11、mesh相对model的变化矩阵
****************************************************/
struct FGltfMeshInfo
{
	FGltfMeshInfo()
	{
		nNumVertices = 0;
		nNumFaces = 0;
		pVertices = nullptr;
		pNormals = nullptr;
		pTextureCoords = nullptr;
		pFacesIndex = nullptr;
		//pBoneIDs = NULL;
		//pBoneWeights = NULL;
		pFacesIndex32 = nullptr;
	}
	//顶点数目
	uint32_t nNumVertices;
	//面数目（三角形数目）
	uint32_t nNumFaces;
	//顶点位置
	Vector3f* pVertices = nullptr;
	//顶点法向量
	Vector3f* pNormals = nullptr;
	//顶点纹理采样坐标
	Vector2f* pTextureCoords = nullptr;
	//切线
	Vector4f* pTangents = nullptr;
	//faces（三角形）索引
	uint16_t* pFacesIndex = nullptr;
	uint32_t* pFacesIndex32 = nullptr;
	int type;
	////骨骼ID
	//CC3DVertexBoneID* pBoneIDs;
	////骨骼权重
	//CC3DVertexBoneWeight* pBoneWeights;
};

class FGltfMesh
{
public:
	FGltfMesh(tinygltf::Model* Model);
	~FGltfMesh();

	void Init(uint32_t MeshIndex,uint32_t PrimitiveIndex, const std::vector < std::shared_ptr<FGltfMaterial>> & ModelMatrial,std::shared_ptr< FGltfNode> ModelNode);
	bool IsTransparent() const;
	const FMatrix& GetMeshMat() const;
	const FBoundingBox& GetBoundingBox()const;
	std::shared_ptr<GltfMeshBuffer> GetGPUBuffer() const;
	std::shared_ptr<FGltfMaterial> GetMaterial() const;
	void Update();
	FMatrix GetPreviousModelMatrix();

private:
	void* Getdata(int32_t attributeIndex, uint32_t& nCount, int32_t& CommpontType);

private:
	std::shared_ptr< FGltfMeshInfo> m_Mesh;
	std::shared_ptr< GltfMeshBuffer> m_GPUBuffer;
	std::string m_MeshName;
	FBoundingBox m_MeshBox;
	tinygltf::Model* m_Model = nullptr;
	std::vector<std::any> m_pData;
	std::shared_ptr<FGltfMaterial> m_Material;
	bool m_isTransparent = false;

	int m_nNodeID = -1;
	int m_nSkinID = -1;

	FMatrix m_MeshMat;
	FMatrix m_PreviousModelMatrix;
};