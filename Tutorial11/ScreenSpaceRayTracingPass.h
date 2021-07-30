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
	void Render(FCommandContext& CommandContext, FDepthBuffer& Depth, FColorBuffer& Albedo,FColorBuffer& Normal);
	void RenderScreenQuad(FCommandContext& CommandContext, FDepthBuffer& Depth, FColorBuffer& Albedo, FColorBuffer& Normal);
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
		FMatrix InvViewProj;
	};
	SSRInfo m_SSRInfo;
	FConstBuffer m_SSRInfoConstBuf;
	D3D12_CPU_DESCRIPTOR_HANDLE m_SSRInfoCpuHandle;

	struct BasePassInfo
	{
		FMatrix projectionMatrix;
		FMatrix modelMatrix;
		FMatrix viewMatrix;
		int mUseTex = 0;
		Vector3i pad;
	};
	BasePassInfo m_BasePassInfo;
	FConstBuffer m_BasePassConstBuf;
	D3D12_CPU_DESCRIPTOR_HANDLE m_BasePassCpuHandle;

	std::vector < std::shared_ptr<FRenderItem>> m_ItemList;
	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_Signature;

	FColorBuffer m_AlbedoBuffer;
	FDepthBuffer m_DepthBuffer;
	Vector2<int> m_GameWndSize;

	DXGI_FORMAT m_RenderTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
};