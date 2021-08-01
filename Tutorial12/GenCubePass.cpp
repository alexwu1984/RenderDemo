#include "GenCubePass.h"
#include "SkyBox.h"
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

void GenCubePass::Init(std::shared_ptr< FSkyBox> skyBox, int32_t width, int height)
{
	m_Size = { width,height };
	m_SkyBox = skyBox;
	SetupRootSignature();
	SetupPipelineState(L"../Resources/Shaders/EnvironmentShaders.hlsl", "VS_LongLatToCube", "PS_LongLatToCube");
}

void GenCubePass::Render(FCubeBuffer& CubeBuffer, FTexture& inputTex)
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

		m_SkyBox->Draw(GfxContext);
	}
	GfxContext.Flush(true);
	FGenerateMips::Generate(CubeBuffer, GfxContext);
	GfxContext.Finish(true);
}

void GenCubePass::SetupRootSignature()
{
	FSamplerDesc PointSamplerDesc(D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
	m_GenCubeSignature.Reset(3, 1);
	m_GenCubeSignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_GenCubeSignature[1].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_GenCubeSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	m_GenCubeSignature.InitStaticSampler(0, PointSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
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
	m_SkyBox->GetMeshLayout(SkyBoxLayout);

	m_RenderState->SetupPipeline(m_GenCubeSignature, SkyBoxLayout);
	m_RenderState->PipelineFinalize();
}
