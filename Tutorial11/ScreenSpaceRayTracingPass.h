#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"

struct FRenderItem;
class FCommandContext;
class RenderPipelineInfo;
class FCamera;

class ScreenSpaceRayTracingPass
{
public:
	ScreenSpaceRayTracingPass();
	~ScreenSpaceRayTracingPass();

	void Init(const std::vector < std::shared_ptr<FRenderItem>>& ItemList, const std::wstring& ShaderFile, int Width, int Height);
	void Render(FCommandContext& CommandContext);
	void Update(const FMatrix& View, const FMatrix& Proj, FCamera& MainCamera);

	FColorBuffer& GetAlbedoBuffer() { return m_AlbedoBuffer; }
	FDepthBuffer& GetDepthBuffer() { return m_DepthBuffer; }

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile);

private:
	std::vector < std::shared_ptr<FRenderItem>> m_ItemList;
	std::shared_ptr< RenderPipelineInfo> m_GBufferRenderState;
	FRootSignature m_Signature;

	FColorBuffer m_AlbedoBuffer;
	FDepthBuffer m_DepthBuffer;
	Vector2<int> m_GameWndSize;
};