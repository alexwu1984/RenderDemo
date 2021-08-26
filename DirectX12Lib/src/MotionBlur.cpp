#include "MotionBlur.h"
#include "CommandContext.h"
#include "BufferManager.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "SamplerManager.h"
#include "D3D12RHI.h"
#include "Camera.h"

using namespace BufferManager;

namespace MotionBlur
{
	FColorBuffer			g_VelocityBuffer;

	FRootSignature			s_RootSignature;

	ComPtr<ID3DBlob>		s_CameraVelocityShader;

	FComputePipelineState	s_CameraVelocityPSO;
}

void MotionBlur::Initialize(void)
{
	uint32_t bufferWidth = g_SceneColorBuffer.GetWidth();
	uint32_t bufferHeight = g_SceneColorBuffer.GetHeight();

	g_VelocityBuffer.Create(L"Motion Vectors", bufferWidth, bufferHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);

	s_RootSignature.Reset(3, 0);
	s_RootSignature[0].InitAsBufferCBV(0, D3D12_SHADER_VISIBILITY_ALL);
	s_RootSignature[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
	s_RootSignature[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);
	s_RootSignature.Finalize(L"Motion Blur");

	s_CameraVelocityShader = D3D12RHI::Get().CreateShader(L"../Resources/Shaders/CameraVelocityCS.hlsl", "cs_main", "cs_5_1");

	s_CameraVelocityPSO.SetRootSignature(s_RootSignature);
	s_CameraVelocityPSO.SetComputeShader(CD3DX12_SHADER_BYTECODE(s_CameraVelocityShader.Get()));
	s_CameraVelocityPSO.Finalize();
}


void MotionBlur::Destroy(void)
{
	g_VelocityBuffer.Destroy();
}
