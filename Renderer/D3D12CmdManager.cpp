#include "D3D12CmdManager.h"


Renderer::D3D12CmdListManager::D3D12CmdListManager()
{
	m_device = D3D12Device::GetDevice();
	for (auto i = 0; i < m_cmdAllocators.size(); ++i)
	{
		m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE(i), MY_IID_PPV_ARGS(&m_cmdAllocators[i]));
	}
}

Renderer::D3D12CmdListManager::~D3D12CmdListManager()
{
}

void Renderer::D3D12CmdListManager::AllocateCmdList(D3D12_COMMAND_LIST_TYPE p_type, 
	ID3D12PipelineState* p_pipelineState,
	REFIID riid,
	_COM_Outptr_  void** ppCommandList)
{
	m_device->CreateCommandList(0, p_type, m_cmdAllocators[p_type], p_pipelineState, riid, ppCommandList);
}
