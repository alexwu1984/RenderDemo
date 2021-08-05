#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"
#include "EnvironmentCommon.h"

struct FRenderItem;
class FCommandContext;
class RenderPipelineInfo;
class FTexture;

class PreIntegratedBRDFPass
{
public:
	PreIntegratedBRDFPass();
	~PreIntegratedBRDFPass();

	void Init();
	void IntegrateBRDF(FColorBuffer& target);


private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint , const std::string& entryPSPoint);

	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_RootSignature;
	Vector2<int> m_Size = { 128,32 };
	int32_t m_CBVRootIndex = 0;
	int32_t m_SRVRootIndex = 1;

};