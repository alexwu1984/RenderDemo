#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "ColorBuffer.h"
#include "EnvironmentContanstBuffer.h"

struct FRenderItem;
class FCommandContext;
class FRenderPipelineInfo;
class FTexture;

class FPreIntegratedBRDFPass
{
public:
	FPreIntegratedBRDFPass();
	~FPreIntegratedBRDFPass();

	void Init();
	void IntegrateBRDF(FColorBuffer& target);


private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint , const std::string& entryPSPoint);

	std::shared_ptr< FRenderPipelineInfo> m_RenderState;
	FRootSignature m_RootSignature;
	Vector2<int> m_Size = { 256,256 };
	int32_t m_CBVRootIndex = 0;
	int32_t m_SRVRootIndex = 1;

};