#pragma once
#include "Inc.h"
#include "MathLib.h"
#include "tiny_gltf.h"

class FGltfMaterial;
class FGltfNode;
class GltfMeshBuffer;
/****************************************************
*	Mesh�ṹ��
*	ÿ��Mesh������
*		1��������Ŀ
*		2������λ��
*		3��������ɫ
*		4�����㷨����
*		5�����������������
*		6��Faces����
*		7��Faces����
*		8��BoneIDs Ӱ��ö���Ĺ���ID
*		9��BoneWeights Ӱ��ö���Ĺ���Ȩ��
*		10��meshʹ�õĲ�������
*		11��mesh���model�ı仯����
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
	//������Ŀ
	uint32_t nNumVertices;
	//����Ŀ����������Ŀ��
	uint32_t nNumFaces;
	//����λ��
	Vector3f* pVertices;
	//���㷨����
	Vector3f* pNormals;
	//���������������
	Vector2f* pTextureCoords;
	//����
	Vector4f* pTangents;
	//faces�������Σ�����
	uint16_t* pFacesIndex;
	uint32_t* pFacesIndex32;
	int type;
	////����ID
	//CC3DVertexBoneID* pBoneIDs;
	////����Ȩ��
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
	bool m_isTransparent = true;

	int m_nNodeID = -1;
	int m_nSkinID = -1;

	FMatrix m_MeshMat;
	FMatrix m_PreviousModelMatrix;
};