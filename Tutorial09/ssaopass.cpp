#include "ssaopass.h"
#include "Model.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "StringUnit.h"
#include "Geometry.h"

SSAOPass::SSAOPass()
{

}

SSAOPass::~SSAOPass()
{

}

void SSAOPass::Init(int Width, int Height)
{
	m_GameWndSize = { Width,Height };
	SetupRootSignature();
	SetupPipelineState();
}

void SSAOPass::Render(FCommandContext& CommandContext, FColorBuffer& PositionBuffer, FColorBuffer& NormalBuffer)
{
	// Set necessary state.
	CommandContext.SetRootSignature(m_RootSignature);
	CommandContext.SetViewportAndScissor(0, 0, m_GameWndSize.x, m_GameWndSize.y);

	RenderWindow& renderWindow = RenderWindow::Get();
	// Indicate that the back buffer will be used as a render target.
	CommandContext.TransitionResource(PositionBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(NormalBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(m_AOBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET,true);

	const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] = { m_AOBuffer.GetRTV() };
	CommandContext.SetRenderTargets(1, RTVs);

	// Record commands.
	CommandContext.ClearColor(m_AOBuffer);
	CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandContext.SetPipelineState(m_RenderState->GetPipelineState());

	CommandContext.SetDynamicDescriptor(0, 0, m_AOPassCpuHandle);
	CommandContext.SetDynamicDescriptor(1, 0, PositionBuffer.GetSRV());
	CommandContext.SetDynamicDescriptor(2, 0, NormalBuffer.GetSRV());
	CommandContext.SetDynamicDescriptor(3, 0, m_NoiseTexture.GetSRV());

	//m_RenderItem->Geo->Draw(CommandContext);
	CommandContext.Draw(3);
}

FColorBuffer& SSAOPass::GetAOBuffer()
{
	return m_AOBuffer;
}

void SSAOPass::SetupRootSignature()
{
	FSamplerDesc DefaultSamplerDesc;
	m_RootSignature.Reset(4, 1);
	m_RootSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
	m_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature.Finalize(L"SSAOPassSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void SSAOPass::SetupPipelineState()
{
	std::shared_ptr<FShader> Shader = FShaderMgr::Get().CreateShader("SSAOPass", L"../Resources/Shaders/Tutorial09/SSAOPass.hlsl");
	m_RenderState = std::make_shared<FRenderPipelineInfo>(Shader);
	const DXGI_FORMAT renderTargetFormat[] = { DXGI_FORMAT_R32G32B32A32_FLOAT };
	m_RenderState->SetupRenderTargetFormat(1, renderTargetFormat, DXGI_FORMAT_UNKNOWN);
	m_RenderState->SetRasterizerState(FPipelineState::RasterizerTwoSided);
	m_RenderState->SetDepthStencilState(FPipelineState::DepthStateDisabled);
	m_RenderState->SetBlendState(FPipelineState::BlendDisable);

	m_RenderState->SetupPipeline(m_RootSignature);
	m_RenderState->PipelineFinalize();

	m_AOBuffer.Create(L"SSAO Buffer", m_GameWndSize.x, m_GameWndSize.y, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);

	m_AOPassConstBuf.CreateUpload(L"SSAOPassInfo", sizeof(SSAOPassInfo));
	m_AOPassCpuHandle = m_AOPassConstBuf.CreateConstantBufferView(0, sizeof(SSAOPassInfo));

	m_PassInfo.KernelSize = 64;
	m_PassInfo.Radius = 1.0;
	m_PassInfo.WindowWidth = m_GameWndSize.x;
	m_PassInfo.WindowHeight = m_GameWndSize.y;
	const float FovVertical = MATH_PI / 4.f;
	m_PassInfo.Proj = FMatrix::MatrixPerspectiveFovLH(FovVertical, (float)m_GameWndSize.x / m_GameWndSize.y, 0.1f, 100.f);
	GenerateKernel();
	
	memcpy(m_AOPassConstBuf.Map(), &m_PassInfo, sizeof(m_PassInfo));

	std::vector<Vector4f> NoiseVev = GenerateNoise();
	m_NoiseTexture.Create(4, 4, DXGI_FORMAT_R32G32B32A32_FLOAT, NoiseVev.data());
}

void SSAOPass::GenerateKernel()
{
	std::uniform_real_distribution<float> RandomFloats(0.0, 1.0); // 随机浮点数，范围0.0 - 1.0
	std::default_random_engine Generator;

	for (unsigned int i = 0; i < 64; ++i)
	{
		Vector3f Sample(RandomFloats(Generator) * 2.0 - 1.0, RandomFloats(Generator) * 2.0 - 1.0, RandomFloats(Generator));
		Sample = Sample.Normalize();
		Sample = Sample * RandomFloats(Generator);
		float Scale = float(i) / 64.0;
		Scale = Lerp(0.1f, 1.0f, Scale * Scale);
		Sample = Sample * Scale;
		m_PassInfo.Samples[i] = Sample;
	}
}

std::vector<Vector4f> SSAOPass::GenerateNoise()
{
	std::uniform_real_distribution<float> RandomFloats(0.0, 1.0); // 随机浮点数，范围0.0 - 1.0
	std::default_random_engine generator;
	std::vector<Vector4f> ssaoNoise;
	ssaoNoise.resize(16);
	for (unsigned int i = 0; i < 16; i++)
	{
		Vector3f noise(RandomFloats(generator) * 2.0 - 1.0, RandomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
		ssaoNoise[i] = std::move(noise);
	}
	return std::move(ssaoNoise);
}
