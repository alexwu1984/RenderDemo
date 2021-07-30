#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "GpuBuffer.h"

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
	void Render(FCommandContext& CommandContext, FDepthBuffer& Depth, FColorBuffer& Albedo);
	void Update(FCamera& MainCamera);

	FColorBuffer& GetAlbedoBuffer() { return m_AlbedoBuffer; }
	FDepthBuffer& GetDepthBuffer() { return m_DepthBuffer; }

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile);

private:
	struct SSRInfo
	{
		float Near = 0.1;
		float Far = 100.f;
		float WindowWidth;
		float WindowHeight;
		Vector3f CameraPosInWorldSpace;
		float RayLength = 10000;
	};
	SSRInfo m_SSRInfo;
	FConstBuffer m_SSRInfoConstBuf;
	D3D12_CPU_DESCRIPTOR_HANDLE m_SSRInfoCpuHandle;

	std::vector < std::shared_ptr<FRenderItem>> m_ItemList;
	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_Signature;

	FColorBuffer m_AlbedoBuffer;
	FDepthBuffer m_DepthBuffer;
	Vector2<int> m_GameWndSize;
};