#pragma once

#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"
#include "EnvironmentCommon.h"

class FCommandContext;
class RenderPipelineInfo;
class FCubeBuffer;
class FSkyBox;
class FTexture;

class GenCubePass
{
public:
	GenCubePass();
	~GenCubePass();

	void Init(std::shared_ptr< FSkyBox> skyBox,int32_t width,int height);
	void Render(FCubeBuffer& CubeBuffer, FTexture& inputTex);

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint);

	std::shared_ptr< FSkyBox> m_SkyBox;
	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_GenCubeSignature;
	Vector2<int> m_Size;
	int32_t m_CBVRootIndex = 0;
	int32_t m_SRVRootIndex = 1;

};