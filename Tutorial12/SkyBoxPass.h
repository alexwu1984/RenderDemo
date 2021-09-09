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
	void Render(FCommandContext& GfxContext, FCamera& MainCamera, FCubeBuffer& CubeBuffer,bool clear);
	void ShowCubeMapDebugView(FCommandContext& GfxContext, FCubeBuffer& CubeBuffer, float Exposure, int MipLevel, const std::vector<Vector3f>& SHCoeffs,int SHDegree);

private:
	void SetupRootSignature();
	void SetupPipelineState(const D3D12_DEPTH_STENCIL_DESC& DepthStencilDesc);

	__declspec(align(16)) struct
	{
		float Exposure;
		int MipLevel;
		int MaxMipLevel;
		int NumSamplesPerDir;
		int Degree;
		Vector3f	CameraPos;
		int		bSHDiffuse;
		Vector3f	pad;
		Vector4f	Coeffs[16];
	} PBR_Constants;

	std::shared_ptr< FModel> m_SkyBox;
	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_SkySignature;
	Vector2<int> m_Size;
	int32_t m_CBVRootIndex = 0;
	int32_t m_SRVRootIndex = 1;


	std::wstring m_ShaderFile;
	std::string m_EntryVSPoint;
	std::string m_EntryPSPoint;

};