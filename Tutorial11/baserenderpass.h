#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"

struct FRenderItem;
class FCommandContext;
class RenderPipelineInfo;
class FCamera;

class BaseRenderPass
{
public:
	BaseRenderPass();
	~BaseRenderPass();

	void Init(const std::vector < std::shared_ptr<FRenderItem>>& ItemList, int Width, int Height);
	void Render(FCommandContext& CommandContext);
	void Update(const Vector3f& LightDir, const FMatrix& View, const FMatrix& Proj, FCamera& MainCamera);

private:
	void SetupRootSignature();
	void SetupPipelineState();

private:
	std::vector < std::shared_ptr<FRenderItem>> m_ItemList;
	std::shared_ptr< RenderPipelineInfo> m_TexutreRenderState;
	FRootSignature m_RootSignature;
	Vector2<int> m_GameWndSize;
};