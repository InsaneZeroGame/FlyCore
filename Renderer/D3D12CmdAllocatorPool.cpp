#include "stdafx.h"
#include "D3D12CmdAllocatorPool.h"
#include "D3D12Device.h"


Renderer::D3D12CmdAllocatorPool::D3D12CmdAllocatorPool():
	m_device(D3D12Device::GetDevice())
{
}

ID3D12CommandAllocator* Renderer::D3D12CmdAllocatorPool::RequestAllocator(D3D12_COMMAND_LIST_TYPE p_type)
{
	ID3D12CommandAllocator* l_allocator = nullptr;

	if (m_readyAllocators[p_type].empty())
	{
		m_device->CreateCommandAllocator(p_type, MY_IID_PPV_ARGS(&l_allocator));
	}
	else
	{
		l_allocator = m_readyAllocators[p_type].front();
		m_readyAllocators[p_type].pop();
	}

	return l_allocator;

}

Renderer::D3D12CmdAllocatorPool::~D3D12CmdAllocatorPool()
{
}
