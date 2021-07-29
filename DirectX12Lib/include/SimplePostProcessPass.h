#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"

struct FRenderItem;
class FCommandContext;
class RenderPipelineInfo;

class SimplePostProcessPass
{
public:
	SimplePostProcessPass();
	~SimplePostProcessPass();

	void Init(const std::wstring& ShaderFile, int Width, int Height);
	void Init(const std::wstring& ShaderFile, const std::string& vs, const std::string& ps,DXGI_FORMAT renderTargetTormat);
	void Render(FCommandContext& CommandContext, const std::function<void(FCommandContext& CommandContext)>& BeforeDrawParam,
		const std::function<void(FCommandContext& CommandContext)>& AfterDrawParam);
	void Render(FColorBuffer& RenderTarget,FCommandContext& CommandContext, const std::function<void(FCommandContext& CommandContext)>& BeforeDrawParam,
		const std::function<void(FCommandContext& CommandContext)>& AfterDrawParam);

	FColorBuffer& GetResult();
	int32_t GetCBVRootIndex() const;
	int32_t GetSRVRootIndex() const;
	void SetViewportAndScissor(int32_t x, int32_t y, int32_t w, int32_t h);

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint="vs_main", const std::string& entryPSPoint="ps_main");

	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_RootSignature;
	Vector2<int> m_GameWndSize;
	Vector2<int> m_Pos;
	FColorBuffer m_PostRenderTarget;
	int32_t m_CBVRootIndex = 0;
	int32_t m_SRVRootIndex = 1;
	DXGI_FORMAT m_RenderTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

};