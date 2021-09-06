#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"

struct FRenderItem;
class FCommandContext;
class RenderPipelineInfo;
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
	void Render(FCommandContext& CommandContext, FCamera& MainCamera,
		FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF);
	void RenderBasePass(FCommandContext& CommandContext, FCamera& MainCamera,
		FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF);
	void RenderIBL(FCommandContext& CommandContext);
	void Update(FCamera& MainCamera);
	void Rotate(float RotateY);

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint);

private:
	std::vector < std::shared_ptr<FRenderItem>> m_ItemList;
	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_MeshSignature;
	Vector2<int> m_GameWndSize;
};