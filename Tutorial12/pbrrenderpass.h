#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"

struct FRenderItem;
class FCommandContext;
class FRenderPipelineInfo;
class FCamera;
class FCubeBuffer;
class FColorBuffer;

class PBRRenderPass
{
public:
	PBRRenderPass();
	~PBRRenderPass();

	void Init(const std::vector < std::shared_ptr<FRenderItem>>& ItemList, int Width, int Height,
		const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint);
	void InitIBL(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint);
	void RenderBasePass(FCommandContext& CommandContext, FCamera& MainCamera,
		FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF,bool Clear);
	void RenderIBL(FCommandContext& GfxContext, FCamera& MainCamera, FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF);
	void Update();
	void Rotate(float RotateY);

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint);

private:
	std::vector < std::shared_ptr<FRenderItem>> m_ItemList;
	std::shared_ptr< FRenderPipelineInfo> m_RenderState;
	std::shared_ptr< FRenderPipelineInfo> m_IBLRenderState;
	FRootSignature m_MeshSignature;
	FRootSignature m_IBLSignature;
	Vector2<int> m_GameWndSize;
};