#include "stdafx.h"
#include "D3D12Texture.h"
#include "D3D12Device.h"
#include "D3D12DescManager.h"
#include "D3D12CmdContext.h"

Renderer::D3D12Texture::D3D12Texture(
	const D3D12_RESOURCE_DESC& p_desc,
	D3D12_HEAP_FLAGS p_flag,
	D3D12_CLEAR_VALUE p_clearValue,
	ID3D12Resource* p_resource):
	m_resourceDesc(p_desc)
{

	if (p_resource)
	{
		m_pResource = p_resource;
		m_UsageState = D3D12_RESOURCE_STATE_COMMON;
	}
	else
	{
		m_UsageState = D3D12_RESOURCE_STATE_COMMON;
		ID3D12Device* l_device = D3D12Device::GetDevice();
		D3D12_HEAP_PROPERTIES l_heapProperties = {};
		l_heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		l_heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		l_heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		l_heapProperties.CreationNodeMask = 0;
		l_heapProperties.VisibleNodeMask = 0;
		ASSERT_SUCCEEDED(l_device->CreateCommittedResource(&l_heapProperties, p_flag, &m_resourceDesc, D3D12_RESOURCE_STATE_COMMON, &p_clearValue, MY_IID_PPV_ARGS(&m_pResource)));
	}
}

Renderer::D3D12Texture::~D3D12Texture()
{
}

Renderer::D3D12DepthBuffer::D3D12DepthBuffer(uint32_t p_width,uint32_t p_height):
	D3D12Texture(
		{D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		p_width,
		p_height,
		1,
		1,
		DXGI_FORMAT_D32_FLOAT,
		{1,0},
		D3D12_TEXTURE_LAYOUT_UNKNOWN ,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL }, 
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		{ DXGI_FORMAT_D32_FLOAT,{1.0f,0} })
{
	CreateViews();
	//Transit resource state to depth_write using graphics cmd context
	auto& l_cmdContxt = D3D12GraphicsCmdContext::GetContext();
	l_cmdContxt.Begin(nullptr);
	l_cmdContxt.TransitResourceState(m_pResource.Get(),
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_DEPTH_WRITE);
	l_cmdContxt.End(true);
}

void Renderer::D3D12DepthBuffer::CreateViews()
{
	D3D12_DEPTH_STENCIL_VIEW_DESC l_viewDesc = {};
	l_viewDesc.Format = m_resourceDesc.Format;
	l_viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	l_viewDesc.Flags = D3D12_DSV_FLAG_NONE;
	m_dsv = D3D12DescManager::GetDescManager().RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	ID3D12Device* l_device = (D3D12Device::GetDevice());
	l_device->CreateDepthStencilView(m_pResource.Get(), &l_viewDesc, m_dsv->cpuHandle);
}

Renderer::D3D12DepthBuffer::~D3D12DepthBuffer()
{
}

Renderer::D3D12RenderTarget::D3D12RenderTarget(uint32_t p_width,uint32_t p_height,ID3D12Resource* p_resource):
	D3D12Texture(
		{
		 D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		 0,
		 p_width,
		 p_height,
		 1,
		 0,
		 Constants::SwapChainFormat,
		{1,
		 0},
		 D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		},
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES,
		{},
		p_resource)
{
	CreateViews();
}

Renderer::D3D12RenderTarget::~D3D12RenderTarget()
{

}

void Renderer::D3D12RenderTarget::CreateViews()
{
	m_rtv = D3D12DescManager::GetDescManager().RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	ID3D12Device* l_device = (D3D12Device::GetDevice());
	l_device->CreateRenderTargetView(m_pResource.Get(), nullptr, m_rtv->cpuHandle);
}
