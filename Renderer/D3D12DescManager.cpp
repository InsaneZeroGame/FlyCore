#include "D3D12DescManager.h"


Renderer::D3D12DescManager::D3D12DescManager()
{
	for (auto i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		m_descHeaps[i] = new D3D12DescHeap(D3D12_DESCRIPTOR_HEAP_TYPE(i));
		ID3D12Device* l_device = D3D12Device::GetDevice();
		l_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE(i));
	}
}

Renderer::D3D12DescManager::~D3D12DescManager()
{
	for (auto i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
	{
		if (m_descHeaps[i])
		{
			delete m_descHeaps[i];
			m_descHeaps[i] = nullptr;
		}
	}
}


