#pragma once

#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"
#include "EnvironmentContanstBuffer.h"

class FCommandContext;
class FRenderPipelineInfo;
class FCubeBuffer;
class FModel;
class FCamera;

class FSkyBoxPass
{
public:
	FSkyBoxPass();
	~FSkyBoxPass();

	void Init(std::shared_ptr< FModel> skyBox,int32_t width,int height,
		const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint);
	void Render(FCommandContext& GfxContext, FCamera& MainCamera, FCubeBuffer& CubeBuffer,bool clear);
	void ShowCubeMapDebugView(FCommandContext& GfxContext, FCubeBuffer& CubeBuffer, float Exposure, int MipLevel, const std::vector<Vector3f>& SHCoeffs,int SHDegree);

private:
	void SetupRootSignature();
	void SetupPipelineState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc);

	std::shared_ptr< FModel> m_SkyBox;
	std::shared_ptr< FRenderPipelineInfo> m_RenderState;
	FRootSignature m_SkySignature;
	Vector2<int> m_Size;
	int32_t m_CBVRootIndex = 0;
	int32_t m_SRVRootIndex = 1;


	std::wstring m_ShaderFile;
	std::string m_EntryVSPoint;
	std::string m_EntryPSPoint;

};