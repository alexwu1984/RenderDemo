#include "hbaopass.h"
#include "Model.h"
#include "SamplerManager.h"
#include "CommandContext.h"
#include "RenderWindow.h"
#include "RenderPipelineInfo.h"
#include "StringUnit.h"
#include "Geometry.h"
#include "DepthBuffer.h"
#include "glm/gtc/random.hpp"
#include <random>

HBAOPass::HBAOPass()
{

}

HBAOPass::~HBAOPass()
{

}

void HBAOPass::Init(int Width, int Height)
{
	m_GameWndSize = { Width,Height };
	SetupRootSignature();
	SetupPipelineState();
}

void HBAOPass::Render(FCommandContext& CommandContext, FDepthBuffer& Depth)
{
	// Set necessary state.
	CommandContext.SetRootSignature(m_RootSignature);
	CommandContext.SetViewportAndScissor(0, 0, m_GameWndSize.x, m_GameWndSize.y);

	RenderWindow& renderWindow = RenderWindow::Get();
	// Indicate that the back buffer will be used as a render target.
	CommandContext.TransitionResource(Depth, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	CommandContext.TransitionResource(m_AOBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET,true);

	const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[] = { m_AOBuffer.GetRTV() };
	CommandContext.SetRenderTargets(1, RTVs);

	// Record commands.
	CommandContext.ClearColor(m_AOBuffer);
	CommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandContext.SetPipelineState(m_RenderState->GetPipelineState());

	CommandContext.SetDynamicDescriptor(0, 0, m_AOPassCpuHandle);
	CommandContext.SetDynamicDescriptor(1, 0, Depth.GetDepthSRV());
	CommandContext.SetDynamicDescriptor(2, 0, m_NoiseTexture.GetSRV());

	CommandContext.Draw(3);
}

FColorBuffer& HBAOPass::GetAOBuffer()
{
	return m_AOBuffer;
}

void HBAOPass::SetupRootSignature()
{
	FSamplerDesc DefaultSamplerDesc;
	m_RootSignature.Reset(3, 1);
	m_RootSignature[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 0, 1, D3D12_SHADER_VISIBILITY_ALL);
	m_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature.InitStaticSampler(0, DefaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSignature.Finalize(L"HBAOPassSignature", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}

void HBAOPass::SetupPipelineState()
{
	std::shared_ptr<FShader> Shader = FShaderMgr::Get().CreateShader("HBAOPass", L"../Resources/Shaders/Tutorial10/HBAOPass.hlsl");
	m_RenderState = std::make_shared<RenderPipelineInfo>(Shader);
	const DXGI_FORMAT renderTargetFormat[] = { DXGI_FORMAT_R32G32B32A32_FLOAT };
	m_RenderState->SetupRenderTargetFormat(1, renderTargetFormat, DXGI_FORMAT_UNKNOWN);
	m_RenderState->SetRasterizerState(FPipelineState::RasterizerTwoSided);
	m_RenderState->SetDepthStencilState(FPipelineState::DepthStateDisabled);
	m_RenderState->SetBlendState(FPipelineState::BlendDisable);

	m_RenderState->SetupPipeline(m_RootSignature);
	m_RenderState->PipelineFinalize();

	m_AOBuffer.Create(L"HBAO Buffer", m_GameWndSize.x, m_GameWndSize.y, 1, DXGI_FORMAT_R32G32B32A32_FLOAT);

	m_AOPassConstBuf.CreateUpload(L"HBAOPassInfo", sizeof(m_PassInfo));
	m_AOPassCpuHandle = m_AOPassConstBuf.CreateConstantBufferView(0, sizeof(m_PassInfo));

	m_PassInfo.WindowWidth = m_GameWndSize.x;
	m_PassInfo.WindowHeight = m_GameWndSize.y;
	m_PassInfo.fov = MATH_PI / 4.f;

	m_PassInfo.FocalLen.x = 1.0f / tanf(m_PassInfo.fov * 0.5f) * ((float)m_PassInfo.WindowHeight / (float)m_PassInfo.WindowWidth);
	m_PassInfo.FocalLen.y = 1.0f / tanf(m_PassInfo.fov * 0.5f);
	
	memcpy(m_AOPassConstBuf.Map(), &m_PassInfo, sizeof(m_PassInfo));

	std::vector<Vector4f> NoiseVev = GenerateNoise();
	m_NoiseTexture.Create(4, 4, DXGI_FORMAT_R32G32B32A32_FLOAT, NoiseVev.data());
}

std::vector<Vector4f> HBAOPass::GenerateNoise()
{
	std::vector<Vector4f> ssaoNoise;
	ssaoNoise.resize(16);
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec2 xy = glm::circularRand(1.0f);
		float z = glm::linearRand(0.0f, 1.0f);
		float w = glm::linearRand(0.0f, 1.0f);

		Vector4f noise(xy[0], xy[1],z,w);
		ssaoNoise[i] = std::move(noise);
	}
	return std::move(ssaoNoise);
}
