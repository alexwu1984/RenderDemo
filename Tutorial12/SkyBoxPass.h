#pragma once

#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"
#include "EnvironmentCommon.h"

class FCommandContext;
class RenderPipelineInfo;
class FCubeBuffer;
class FModel;
class FCamera;

class SkyBoxPass
{
public:
	SkyBoxPass();
	~SkyBoxPass();

	void Init(std::shared_ptr< FModel> skyBox,int32_t width,int height,
		const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint);
	void Render(FCommandContext& GfxContext, FCamera& MainCamera, FCubeBuffer& CubeBuffer);
	void ShowCubeMapDebugView(FCommandContext& GfxContext, FCubeBuffer& CubeBuffer, float Exposure, int MipLevel, const std::vector<Vector3f>& SHCoeffs,int SHDegree);

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint);

	std::shared_ptr< FModel> m_SkyBox;
	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_SkySignature;
	Vector2<int> m_Size;
	int32_t m_CBVRootIndex = 0;
	int32_t m_SRVRootIndex = 1;

};