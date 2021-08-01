#pragma once

#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"
#include "EnvironmentCommon.h"

struct FRenderItem;
class FCommandContext;
class RenderPipelineInfo;
class FCubeBuffer;
class FSkyBox;
class FCamera;

class SkyBoxPass
{
public:
	SkyBoxPass();
	~SkyBoxPass();

	void Init(std::shared_ptr< FSkyBox> skyBox,int32_t width,int height);
	void Render(FCommandContext& GfxContext, FCamera& MainCamera, FCubeBuffer& CubeBuffer);

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint);

	std::shared_ptr< FSkyBox> m_SkyBox;
	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_SkySignature;
	Vector2<int> m_Size;
	int32_t m_CBVRootIndex = 0;
	int32_t m_SRVRootIndex = 1;

};