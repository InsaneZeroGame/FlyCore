#include "D3D12DescHeap.h"

Renderer::D3D12DescHeap::D3D12DescHeap(
	D3D12_DESCRIPTOR_HEAP_TYPE p_type,
	uint32_t  p_descNum,
	D3D12_DESCRIPTOR_HEAP_FLAGS p_flag):
	m_device(D3D12Device::GetDevice()),
	m_descHeap(nullptr)
{
	D3D12_DESCRIPTOR_HEAP_DESC l_desc = {};
	l_desc.Flags = p_flag;
	l_desc.NodeMask = 0;
	l_desc.NumDescriptors = p_descNum;
	l_desc.Type = p_type;

	ASSERT_SUCCEEDED(m_device->CreateDescriptorHeap(&l_desc, MY_IID_PPV_ARGS(&m_descHeap)));
}

Renderer::D3D12DescHeap::~D3D12DescHeap()
{
}
