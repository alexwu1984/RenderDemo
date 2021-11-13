#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"

struct FRenderItem;
class FCommandContext;
class FRenderPipelineInfo;
class FCamera;

class GBufferRenderPass
{
public:
	GBufferRenderPass();
	~GBufferRenderPass();

	void Init(const std::vector < std::shared_ptr<FRenderItem>>& ItemList, const std::wstring& ShaderFile, int Width, int Height);
	void Render(FCommandContext& CommandContext);
	void Update(const Vector3f& LightDir, const FMatrix& LightView, const FMatrix& LightProj, FCamera& MainCamera);

	FColorBuffer& GetAlbedoBuffer() { return m_AlbedoBuffer; }
	FColorBuffer& GetNormalBuffer() { return m_NormalBuffer; }
	FColorBuffer& GetPositionBuffer() { return m_PositionBuffer; }
	FDepthBuffer& GetDepthBuffer() { return m_DepthBuffer; }

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile);

private:
	std::vector < std::shared_ptr<FRenderItem>> m_ItemList;
	std::shared_ptr< FRenderPipelineInfo> m_GBufferRenderState;
	FRootSignature m_GBufferSignature;

	//DXGI_FORMAT m_RenderTargetFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	DXGI_FORMAT m_RenderTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	FColorBuffer m_AlbedoBuffer;
	FColorBuffer m_NormalBuffer;
	FColorBuffer m_PositionBuffer;
	FDepthBuffer m_DepthBuffer;
	Vector2<int> m_GameWndSize;
};