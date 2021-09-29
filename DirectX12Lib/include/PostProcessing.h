#pragma once

class FCamera;
class FCubeBuffer;
class FColorBuffer;
class FCommandContext;

namespace PostProcessing
{
	extern bool g_EnableBloom;
	extern float g_BloomIntensity;
	extern float g_BloomThreshold;
	extern bool g_EnableSSR;

	void Initialize();
	void Destroy();

	void Render(FCommandContext& GfxContext);
	void GenerateBloom(FCommandContext& GfxContext);
	void ToneMapping(FCommandContext& GfxContext);

	void BuildHZB(FCommandContext& GfxContext);
	void GenerateSSR(FCommandContext& GfxContext, FCamera& Camera, FCubeBuffer& CubeBuffer);
}