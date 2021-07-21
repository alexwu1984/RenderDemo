#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"

struct FRenderItem;
class FCommandContext;
class RenderPipelineInfo;

class SamplePostProcessPass
{
public:
	SamplePostProcessPass();
	~SamplePostProcessPass();

	void Init(const std::wstring& ShaderFile, int Width, int Height);
	void Render(FCommandContext& CommandContext, const std::function<void(FCommandContext& CommandContext)>& BeforeDrawParam,
		const std::function<void(FCommandContext& CommandContext)>& AfterDrawParam);
	void Render(FColorBuffer& RenderTarget,FCommandContext& CommandContext, const std::function<void(FCommandContext& CommandContext)>& BeforeDrawParam,
		const std::function<void(FCommandContext& CommandContext)>& AfterDrawParam);

	FColorBuffer& GetResult();
	int32_t GetCBVRootIndex() const;
	int32_t GetSRVRootIndex() const;

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile);

	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_RootSignature;
	Vector2<int> m_GameWndSize;
	FColorBuffer m_PostRenderTarget;
	int32_t m_CBVRootIndex = 0;
	int32_t m_SRVRootIndex = 1;

};