#include "D3D12DescHeap.h"

Renderer::D3D12DescHeap::D3D12DescHeap(
	D3D12_DESCRIPTOR_HEAP_TYPE p_type,
	uint32_t  p_descNum,
	D3D12_DESCRIPTOR_HEAP_FLAGS p_flag):
	m_device(D3D12Device::GetDevice()),
	m_descHeap(nullptr),
	m_descHandleIncrementSize(0),
	m_maxDescCount(p_descNum),
	m_descOffset(0),
	m_allocatedCount(0),
	m_type(p_type)
{
	D3D12_DESCRIPTOR_HEAP_DESC l_desc = {};
	l_desc.Flags = p_flag;
	l_desc.NodeMask = 0;
	l_desc.NumDescriptors = p_descNum;
	l_desc.Type = p_type;

	ASSERT_SUCCEEDED(m_device->CreateDescriptorHeap(&l_desc, MY_IID_PPV_ARGS(&m_descHeap)));
	m_descHandleIncrementSize = m_device->GetDescriptorHandleIncrementSize(p_type);
}

Renderer::D3D12DescHeap::~D3D12DescHeap()
{
	for (auto i = 0; i < m_descs.size(); ++i)
	{
		auto* l_desc = m_descs.front();
		SAFE_DELETE(l_desc);
		m_descs.pop();
	}
}

const Renderer::D3D12Descriptor* Renderer::D3D12DescHeap::RequestDesc()
{
	if (!m_descs.empty())
	{
		//Has available descs
		const D3D12Descriptor* l_desc = m_descs.front();
		m_descs.pop();
		return l_desc;
	}
	else
	{
		//Need to allocate new one
		if (m_allocatedCount < m_maxDescCount)
		{
			//Exceed Maximum
			auto* l_desc = new D3D12Descriptor;
			l_desc->type = m_type;
			l_desc->cpuHandle.ptr = m_descHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_descOffset;
			l_desc->gpuHandle.ptr = m_descHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_descOffset;
			m_descOffset += m_descHandleIncrementSize;
			m_allocatedCount++;
			return l_desc;
		}
		else
		{
			return nullptr;
		}
	}
}
