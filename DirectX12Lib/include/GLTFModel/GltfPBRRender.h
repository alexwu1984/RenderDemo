#pragma once
#include "Inc.h"
#include "RootSignature.h"
#include "MathLib.h"
#include "GLTFModel/PBRMaterial.h"

class FRenderPipelineInfo;
class FCommandContext;
class FCamera;
class FCubeBuffer;
class FColorBuffer;
class FGLTFMode;

class FGlftPBRRender
{
public:
	FGlftPBRRender();
	~FGlftPBRRender();

	void InitBase(std::shared_ptr<FGLTFMode> Mode, int Width, int Height,
		const std::wstring& ShaderFile, const std::string& EntryVSPoint, const std::string& EntryPSPoint);
	void InitIBL(const std::wstring& ShaderFile, const std::string& EntryVSPoint, const std::string& EntryPSPoint);
	void RenderBasePass(FCommandContext& CommandContext, FCamera& MainCamera,
		FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF, bool Clear);
	void RenderIBL(FCommandContext& GfxContext, FCamera& MainCamera, FCubeBuffer& IrradianceCube, FCubeBuffer& PrefilteredCube, FColorBuffer& PreintegratedGF);

private:
	void SetupBaseRootSignature();
	void SetupBasePipelineState(const std::wstring& ShaderFile, const std::string& EntryVSPoint, const std::string& EntryPSPoint);

private:
	std::shared_ptr<FGLTFMode> m_GltfMode;
	std::shared_ptr< FRenderPipelineInfo> m_RenderState;
	std::shared_ptr< FRenderPipelineInfo> m_IBLRenderState;
	FRootSignature m_MeshSignature;
	FRootSignature m_IBLSignature;
	Vector2<int> m_GameWndSize;
	FPBRPSConstants m_IBLPS;
};