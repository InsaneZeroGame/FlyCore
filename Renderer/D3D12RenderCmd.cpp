#include "stdafx.h"
#include "D3D12RenderCmd.h"


Renderer::D3D12CmdQueue* Renderer::D3D12GraphicsCmd::m_cmdQueue = nullptr;
Renderer::D3D12CmdQueue* Renderer::D3D12ComputeCmd::m_cmdQueue = nullptr;

Renderer::D3D12Cmd::D3D12Cmd(D3D12_COMMAND_LIST_TYPE p_type, uint32_t p_cmdListCount):
	m_cmdAllocatorPool(D3D12CmdAllocatorPool::GetPoolPtr()),
	m_cmdListManager(D3D12CmdListManager::GetManagerPtr())
{
	m_cmdAllocator.resize(p_cmdListCount);
	for (uint32_t i = 0; i < p_cmdListCount; ++i)
	{
		m_cmdAllocator[i] = m_cmdAllocatorPool->RequestAllocator(p_type);
	}
}


Renderer::D3D12Cmd::~D3D12Cmd()
{
}

Renderer::D3D12GraphicsCmd::D3D12GraphicsCmd(const uint32_t& p_count):
	D3D12Cmd(D3D12_COMMAND_LIST_TYPE_DIRECT, p_count)
{
	m_cmdQueue = new D3D12CmdQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_cmdListManager->AllocateCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT, nullptr, m_cmdAllocator[0], MY_IID_PPV_ARGS(&m_cmdList));
	m_cmdListManager->CloseCmdList(m_cmdList);
}

Renderer::D3D12GraphicsCmd::~D3D12GraphicsCmd()
{
	SAFE_DELETE(m_cmdQueue);
}

void Renderer::D3D12GraphicsCmd::Reset(const uint32_t& p_index, ID3D12PipelineState* p_state) const
{
	m_cmdAllocator[p_index]->Reset();
	m_cmdListManager->ResetCmdList(m_cmdList, m_cmdAllocator[p_index], p_state);

}

void Renderer::D3D12GraphicsCmd::Flush(bool p_waitFinish) const
{
	ID3D12CommandList* ppCommandLists[] = { m_cmdList };
	m_cmdQueue->Flush(_countof(ppCommandLists), ppCommandLists);
	if(p_waitFinish)
		m_cmdQueue->WaitFinish();
}

Renderer::D3D12ComputeCmd::D3D12ComputeCmd(const uint32_t& p_count):
	D3D12Cmd(D3D12_COMMAND_LIST_TYPE_COMPUTE,p_count),
	m_cmdList(nullptr)
{
	m_cmdQueue = new D3D12CmdQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	m_cmdListManager->AllocateCmdList(D3D12_COMMAND_LIST_TYPE_COMPUTE, nullptr, m_cmdAllocator[0], MY_IID_PPV_ARGS(&m_cmdList));
}

Renderer::D3D12ComputeCmd::~D3D12ComputeCmd()
{
	SAFE_DELETE(m_cmdQueue);
}
