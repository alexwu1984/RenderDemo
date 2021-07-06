#include "ShadowMap.h"
#include "d3dx12.h"
#include "D3D12RHI.h"

FShadowMap::FShadowMap(uint32_t width, uint32_t height)
{
	m_Size.x = width;
	m_Size.y = height;

	mViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)height };

	m_ShadowMap.Create(L"shadowmap", width, height, DXGI_FORMAT_D24_UNORM_S8_UINT);

	//D3D12_RESOURCE_DESC TexDesc;
	//ZeroMemory(&TexDesc, sizeof(D3D12_RESOURCE_DESC));
	//TexDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	//TexDesc.Alignment = 0;
	//TexDesc.Width = m_Size.x;
	//TexDesc.Height = m_Size.y;
	//TexDesc.DepthOrArraySize = 1;
	//TexDesc.MipLevels = 1;
	//TexDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	//TexDesc.SampleDesc.Count = 1;
	//TexDesc.SampleDesc.Quality = 0;
	//TexDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	//TexDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//D3D12_CLEAR_VALUE optClear;
	//optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//optClear.DepthStencil.Depth = 1.0f;
	//optClear.DepthStencil.Stencil = 0;

	//ComPtr<ID3D12Resource> shadowMap;
	//ThrowIfFailed(D3D12RHI::Get().GetD3D12Device()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//	D3D12_HEAP_FLAG_NONE, &TexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&shadowMap)));

	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//srvDesc.Texture2D.MostDetailedMip = 0;
	//srvDesc.Texture2D.MipLevels = 1;
	//srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	//srvDesc.Texture2D.PlaneSlice = 0;
	//m_CpuSrv = D3D12RHI::Get().AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//D3D12RHI::Get().GetD3D12Device()->CreateShaderResourceView(shadowMap.Get(), &srvDesc, m_CpuSrv);

	//// Create DSV to resource so we can render to the shadow map.
	//D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	//dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	//dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//dsvDesc.Texture2D.MipSlice = 0;
	//m_CpuDsv = D3D12RHI::Get().AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//D3D12RHI::Get().GetD3D12Device()->CreateDepthStencilView(shadowMap.Get(), &dsvDesc, m_CpuDsv);

	//m_ShadowMap = FD3D12Resource(shadowMap.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

uint32_t FShadowMap::Width() const
{
	return m_Size.x;
}

uint32_t FShadowMap::Height() const
{
	return m_Size.y;
}

const D3D12_CPU_DESCRIPTOR_HANDLE& FShadowMap::Srv()
{
	return m_ShadowMap.GetDepthSRV();
}

const D3D12_CPU_DESCRIPTOR_HANDLE& FShadowMap::Dsv()
{
	return m_ShadowMap.GetDSV();
}

FDepthBuffer& FShadowMap::GetBuffer()
{
	return m_ShadowMap;
}

D3D12_VIEWPORT FShadowMap::Viewport() const
{
	return mViewport;
}

D3D12_RECT FShadowMap::ScissorRect() const
{
	return mScissorRect;
}
