#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"

class FCommandContext;
class RenderPipelineInfo;

class ScreenQuadRenderPass
{
public:
	ScreenQuadRenderPass();
	~ScreenQuadRenderPass();

	void Init(const std::wstring& ShaderFile, int Width, int Height);
	void Render(FCommandContext& CommandContext,const std::function<void(FCommandContext& CommandContext)> &BeforeDrawParam,
		const std::function<void(FCommandContext& CommandContext)>& AfterDrawParam);

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile);

	std::shared_ptr< RenderPipelineInfo> m_ScreenQuadRenderState;
	FRootSignature m_ScreenQuadSignature;
	Vector2<int> m_GameWndSize;
};