#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "Texture.h"

struct FRenderItem;
class FCommandContext;
class RenderPipelineInfo;
class FCamera;
class FCubeBuffer;
class FColorBuffer;

class PBRFloorRenderPass
{
public:
	PBRFloorRenderPass();
	~PBRFloorRenderPass();

	void Init();
	void RenderBasePass(FCommandContext& CommandContext);

private:
	void SetupRootSignature();
	void SetupPipelineState();

private:
	std::vector < std::shared_ptr<FRenderItem>> m_ItemList;
	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_MeshSignature;
	FTexture m_FloorAlbedo;
	FTexture m_FloorAlpha;
};