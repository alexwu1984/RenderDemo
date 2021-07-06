#pragma once
#include "Common.h"
#include "MathLib.h"
#include "DepthBuffer.h"

class FShadowMap
{
public:
	FShadowMap(uint32_t width, uint32_t height);
	~FShadowMap() {}

	FShadowMap(const FShadowMap& rhs) = delete;
	FShadowMap& operator=(const FShadowMap& rhs) = delete;

	uint32_t Width()const;
	uint32_t Height()const;

	const D3D12_CPU_DESCRIPTOR_HANDLE& Srv();
	const D3D12_CPU_DESCRIPTOR_HANDLE& Dsv();

	FDepthBuffer& GetBuffer();

	D3D12_VIEWPORT Viewport()const;
	D3D12_RECT ScissorRect()const;

private:
	Vector2ui m_Size;
	FDepthBuffer m_ShadowMap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CpuSrv;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CpuDsv;
	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
};