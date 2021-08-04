#include "GenCubePass.h"
#include "Model.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "StringUnit.h"
#include "Shader.h"
#include "CubeBuffer.h"
#include "GenerateMips.h"

GenCubePass::GenCubePass()
{

}

GenCubePass::~GenCubePass()
{

}

void GenCubePass::Init(std::shared_ptr< FModel> skyBox, int32_t width, int height,
	const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint, CubePass passType)
{
	m_passType = passType;
	m_Size = { width,height };
	m_Cube = skyBox;
	SetupRootSignature();
	SetupPipelineState(ShaderFile, entryVSPoint, entryPSPoint);
}

void GenCubePass::GenerateCubeMap(FCubeBuffer& CubeBuffer, FTexture& inputTex)
{
	FCommandContext& GfxContext = FCommandContext::Begin(D3D12_COMMAND_LIST_TYPE_DIRECT, L"3D Queue");

	GfxContext.SetRootSignature(m_GenCubeSignature);
	GfxContext.SetPipelineState(m_RenderState->GetPipelineState());
	GfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GfxContext.SetViewportAndScissor(0, 0, m_Size.x, m_Size.y); // very important, cubemap size

	GfxContext.TransitionResource(CubeBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

	GfxContext.SetDynamicDescriptor(2, 0, inputTex.GetSRV());

	g_EVSConstants.ModelMatrix = FMatrix(); // identity

	for (int i = 0; i < 6; ++i)
	{
		GfxContext.SetRenderTargets(1, &CubeBuffer.GetRTV(i, 0));
		GfxContext.ClearColor(CubeBuffer, i, 0);

		g_EVSConstants.ViewProjMatrix = CubeBuffer.GetViewProjMatrix(i);
		GfxContext.SetDynamicConstantBufferView(0, sizeof(g_EVSConstants), &g_EVSConstants);

		m_Cube->Draw(GfxContext);
	}
	GfxContext.Flush(true);
	FGenerateMips::Generate(CubeBuffer, GfxContext);
	GfxContext.Finish(true);
}

void GenCubePass::GenerateIrradianceMap(FCubeBuffer& CubeBuffer, FCubeBuffer& IrradianceCube, int NumSamplesPerDir)
{
	FCommandContext& GfxContext = FCommandContext::Begin(D3D12_COMMAND_LIST_TYPE_DIRECT, L"3D Queue");

	GfxContext.SetRootSignature(m_GenCubeSignature);
	GfxContext.SetPipelineState(m_RenderState->GetPipelineState());
	GfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	GfxContext.SetViewportAndScissor(0, 0, m_Size.x, m_Size.y); // very important, cubemap size

	GfxContext.TransitionResource(CubeBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(IrradianceCube, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

	GfxContext.SetDynamicDescriptor(2, 0, CubeBuffer.GetCubeSRV());

	g_EPSConstants.NumSamplesPerDir = NumSamplesPerDir;
	GfxContext.SetDynamicConstantBufferView(1, sizeof(g_EPSConstants), &g_EPSConstants);

	g_EVSConstants.ModelMatrix = FMatrix(); // identity
	for (int i = 0; i < 6; ++i)
	{
		GfxContext.SetRenderTargets(1, &IrradianceCube.GetRTV(i, 0));
		GfxContext.ClearColor(IrradianceCube, i, 0);

		g_EVSConstants.ViewProjMatrix = IrradianceCube.GetViewProjMatrix(i);
		GfxContext.SetDynamicConstantBufferView(0, sizeof(g_EVSConstants), &g_EVSConstants);

		m_Cube->Draw(GfxContext);
	}
	GfxContext.Finish(true);
}

void GenCubePass::GeneratePrefilteredMap(FCubeBuffer& CubeBuffer, FCubeBuffer& PrefilteredCube)
{
	FCommandContext& GfxContext = FCommandContext::Begin(D3D12_COMMAND_LIST_TYPE_DIRECT, L"3D Queue");

	GfxContext.SetRootSignature(m_GenCubeSignature);
	GfxContext.SetPipelineState(m_RenderState->GetPipelineState());
	GfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	GfxContext.TransitionResource(CubeBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	GfxContext.TransitionResource(PrefilteredCube, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

	GfxContext.SetDynamicDescriptor(2, 0, CubeBuffer.GetCubeSRV());

	g_EVSConstants.ModelMatrix = FMatrix(); // identity
	uint32_t NumMips = PrefilteredCube.GetNumMips();
	g_EPSConstants.MaxMipLevel = NumMips;
	for (uint32_t MipLevel = 0; MipLevel < NumMips; ++MipLevel)
	{
		uint32_t Size = m_Size.x >> MipLevel;
		GfxContext.SetViewportAndScissor(0, 0, Size, Size);

		g_EPSConstants.MipLevel = MipLevel;
		GfxContext.SetDynamicConstantBufferView(1, sizeof(g_EPSConstants), &g_EPSConstants);

		for (int i = 0; i < 6; ++i)
		{
			GfxContext.SetRenderTargets(1, &PrefilteredCube.GetRTV(i, MipLevel));
			GfxContext.ClearColor(PrefilteredCube, i, MipLevel);

			g_EVSConstants.ViewProjMatrix = PrefilteredCube.GetViewProjMatrix(i);
			GfxContext.SetDynamicConstantBufferView(0, sizeof(g_EVSConstants), &g_EVSConstants);

			m_Cube->Draw(GfxContext);
		}
	}

	GfxContext.Finish(true);
}

void GenCubePass::SetupRootSignature()
{
	FSamplerDesc PointSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	FSamplerDesc DefaultSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	m_GenCubeSignature.Reset(3, 1);
	m_GenCubeSignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_GenCubeSignature[1].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_GenCubeSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	if (m_passType == CubePass_CubeMap)
	{
		m_GenCubeSignature.InitStaticSampler(0, PointSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	}
	else
	{
		m_GenCubeSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	}
	
	m_GenCubeSignature.Finalize(L"GenCubePass", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void GenCubePass::SetupPipelineState(const std::wstring& ShaderFile, const std::string& entryVSPoint, const std::string& entryPSPoint)
{
	std::shared_ptr<FShader> Shader = FShaderMgr::Get().CreateShaderDirect(ShaderFile, entryVSPoint, entryPSPoint);
	m_RenderState = std::make_shared<RenderPipelineInfo>(Shader);
	DXGI_FORMAT CubeFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	m_RenderState->SetupRenderTargetFormat(1, &CubeFormat, DXGI_FORMAT_UNKNOWN);
	m_RenderState->SetRasterizerState(FPipelineState::RasterizerTwoSided);
	m_RenderState->SetDepthStencilState(FPipelineState::DepthStateDisabled);
	m_RenderState->SetBlendState(FPipelineState::BlendDisable);

	std::vector<D3D12_INPUT_ELEMENT_DESC> SkyBoxLayout;
	m_Cube->GetMeshLayout(SkyBoxLayout);

	m_RenderState->SetupPipeline(m_GenCubeSignature, SkyBoxLayout);
	m_RenderState->PipelineFinalize();
}
