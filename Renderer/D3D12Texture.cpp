#include "D3D12Texture.h"
#include "D3D12Device.h"


Renderer::D3D12Texture::D3D12Texture(
	D3D12_RESOURCE_DESC* p_desc,
	D3D12_HEAP_FLAGS p_flag,
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
		ASSERT_SUCCEEDED(l_device->CreateCommittedResource(&l_heapProperties, p_flag, m_resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, MY_IID_PPV_ARGS(&m_pResource)));

	}
	}

Renderer::D3D12Texture::~D3D12Texture()
{
}
