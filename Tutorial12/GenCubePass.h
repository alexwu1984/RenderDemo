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
class FTexture;

class GenCubePass
{
public:
	enum CubePass
	{
		CubePass_CubeMap,
		CubePass_IrradianceMap,
		CubePass_PreFilterEnvMap,
	};
public:
	GenCubePass();
	~GenCubePass();

	void Init(std::shared_ptr< FModel> skyBox,int32_t width,int height, 
		const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint, CubePass passType);
	void GenerateCubeMap(FCubeBuffer& CubeBuffer, FTexture& inputTex);
	void GenerateIrradianceMap(FCubeBuffer& CubeBuffer,FCubeBuffer& IrradianceCube,int NumSamplesPerDir);
	void GeneratePrefilteredMap(FCubeBuffer& CubeBuffer, FCubeBuffer& PrefilteredCube);

private:
	void SetupRootSignature();
	void SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint);

	CubePass m_passType;
	std::shared_ptr< FModel> m_Cube;
	std::shared_ptr< RenderPipelineInfo> m_RenderState;
	FRootSignature m_GenCubeSignature;
	Vector2<int> m_Size;
	int32_t m_CBVRootIndex = 0;
	int32_t m_SRVRootIndex = 1;

};