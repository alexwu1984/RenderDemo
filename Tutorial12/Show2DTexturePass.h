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

class Show2DTexturePass
{
public:
	Show2DTexturePass();
	~Show2DTexturePass();

	void Init();
	void ShowTexture2D(FCommandContext& GfxContext, FTexture& inputTex);

	void SetViewportAndScissor(int32_t x, int32_t y, int32_t w, int32_t h);

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint , const std::string& entryPSPoint);

	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_RootSignature;
	Vector2<int> m_Size;
	Vector2<int> m_Pos;
	int32_t m_CBVRootIndex = 0;
	int32_t m_SRVRootIndex = 1;

};