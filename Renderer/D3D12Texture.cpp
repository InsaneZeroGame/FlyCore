#include "stdafx.h"
#include "D3D12Texture.h"
#include "D3D12Device.h"
#include "D3D12DescManager.h"
#include "D3D12CmdContext.h"

Renderer::D3D12Texture::D3D12Texture(
	const D3D12_RESOURCE_DESC& p_desc,
	D3D12_HEAP_FLAGS p_flag,
	D3D12_CLEAR_VALUE p_clearValue,
	ID3D12Resource* p_resource,
	D3D12_RESOURCE_STATES p_state):
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
		ASSERT_SUCCEEDED(l_device->CreateCommittedResource(&l_heapProperties, p_flag, &m_resourceDesc, p_state, &p_clearValue, MY_IID_PPV_ARGS(&m_pResource)));
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
		Constants::DepthFormat,
		{1,0},
		D3D12_TEXTURE_LAYOUT_UNKNOWN ,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL }, 
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		{ Constants::DepthFormat,{1.0f,0} })
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


	D3D12_SHADER_RESOURCE_VIEW_DESC l_srvDesc = {};
	l_srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	l_srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	l_srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	l_srvDesc.Texture2D = {0,1,0,0};
	m_srv = D3D12DescManager::GetDescManager().RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	l_device->CreateShaderResourceView(m_pResource.Get(), &l_srvDesc, m_srv->cpuHandle);
}

Renderer::D3D12DepthBuffer::~D3D12DepthBuffer()
{
}

Renderer::D3D12RenderTarget::D3D12RenderTarget(uint32_t p_width,uint32_t p_height,ID3D12Resource* p_resource,DXGI_FORMAT p_format):
	D3D12Texture(
		{
		 D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		 0,
		 p_width,
		 p_height,
		 1,
		 1,
		 p_format,
		{1,
		 0},
		 D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		},
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		{p_format},
		p_resource, D3D12_RESOURCE_STATE_RENDER_TARGET)
		
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

	m_srv = D3D12DescManager::GetDescManager().RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	l_device->CreateShaderResourceView(m_pResource.Get(), nullptr, m_srv->cpuHandle);
}

Renderer::D3D12Texture2D::D3D12Texture2D(uint32_t p_width, uint32_t p_height, uint32_t p_depth,DXGI_FORMAT p_format)
	:D3D12Texture(
		{
		 D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		 0,
		 p_width,
		 p_height,
		 (UINT16)p_depth,
		 1,
		 p_format,
		{1,
		 0},
		 D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		},
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		{ p_format },
		nullptr, D3D12_RESOURCE_STATE_COPY_DEST)
{
	CreateViews();
}

Renderer::D3D12Texture2D::~D3D12Texture2D()
{
}

void Renderer::D3D12Texture2D::CreateViews()
{
	ID3D12Device* l_device = (D3D12Device::GetDevice());
	m_srv = D3D12DescManager::GetDescManager().RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	l_device->CreateShaderResourceView(m_pResource.Get(), nullptr, m_srv->cpuHandle);

}

Renderer::D3D12TextureCube::D3D12TextureCube(uint32_t p_width, uint32_t p_height, DXGI_FORMAT p_format):
	D3D12Texture(
		{
		 D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		 0,
		 p_width,
		 p_height,
		 6,
		 1,
		 p_format,
		{1,
		 0},
		 D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		},
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		{ p_format },
		nullptr, D3D12_RESOURCE_STATE_COPY_DEST)
{
	CreateViews();
}

Renderer::D3D12TextureCube::~D3D12TextureCube()
{
}

void Renderer::D3D12TextureCube::CreateViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC l_cubeSRV = {};
	l_cubeSRV.Format = m_resourceDesc.Format;
	l_cubeSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	l_cubeSRV.TextureCube = {0,1,0};
	l_cubeSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	ID3D12Device* l_device = (D3D12Device::GetDevice());
	m_srv = D3D12DescManager::GetDescManager().RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	l_device->CreateShaderResourceView(m_pResource.Get(), &l_cubeSRV, m_srv->cpuHandle);
}

