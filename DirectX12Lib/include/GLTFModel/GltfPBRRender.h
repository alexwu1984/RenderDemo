#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "GLTFModel/PBRMaterial.h"

class FRenderPipelineInfo;
class FCommandContext;
class FCamera;
class FCubeBuffer;
class FColorBuffer;
class FGLTFMode;
class FGltfMesh;

class FGlftPBRRender
{
public:
	FGlftPBRRender();
	~FGlftPBRRender();

	void InitBase(std::shared_ptr<FGLTFMode> Mode, int Width, int Height,
		const std::wstring& ShaderFile, const std::string& EntryVSPoint, const std::string& EntryPSPoint);
	void InitIBL(const std::wstring& ShaderFile, const std::string& EntryVSPoint, const std::string& EntryPSPoint);
	void RenderBasePass(FCommandContext& CommandContext, FCamera& MainCamera,
		FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF, bool Clear);
	void RenderIBL(FCommandContext& GfxContext, FCamera& MainCamera, FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF);
	void Rotate(float RotateY);

private:
	void SetupBaseRootSignature();
	void SetupBasePipelineState(const std::wstring& ShaderFile, const std::string& EntryVSPoint, const std::string& EntryPSPoint);
	void RenderMesh(std::shared_ptr<FGltfMesh> GltfMesh, FCamera& MainCamera, FCommandContext& CommandContext,
		FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF);

	//对Mesh进行排序，按顺序渲染
	void GetSortMeshID();
	void GetBoxPoint(Vector3f& minPoint, Vector3f& maxPoint);

	struct MeshDisInfo
	{
		float Distance;
		int MeshID;
		int ModelID;
		//区分mesh包围框的最近最远点
		int PosType;

		bool operator()(const MeshDisInfo& pNear, const MeshDisInfo& pFar)
		{
			return pNear.Distance > pFar.Distance;
		}
	};

private:
	std::shared_ptr<FGLTFMode> m_GltfMode;
	std::shared_ptr< FRenderPipelineInfo> m_RenderState;
	std::shared_ptr< FRenderPipelineInfo> m_BlendRenderStateBack;
	std::shared_ptr< FRenderPipelineInfo> m_BlendRenderStateFront;
	std::shared_ptr< FRenderPipelineInfo> m_IBLRenderState;
	FRootSignature m_MeshSignature;
	FRootSignature m_IBLSignature;
	Vector2f m_GameWndSize;
	FPBRPSConstants m_IBLPS;

	std::vector<MeshDisInfo> m_SortMesh;
	std::vector<Vector3f> m_BoxPoint;
	Vector4f m_CameraPos;
};