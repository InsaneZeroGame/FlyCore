#include "D3D12CmdQueue.h"
#include "D3D12Device.h"


Renderer::D3D12CmdQueue::D3D12CmdQueue(D3D12_COMMAND_LIST_TYPE p_type):
	m_device(D3D12Device::GetDevice()),
	m_fenceValue(1),
	m_fenceEvent(nullptr),
	m_fence(nullptr)
{
	D3D12_COMMAND_QUEUE_DESC l_desc = {};
	l_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	l_desc.NodeMask = 0;
	l_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	l_desc.Type = p_type;
	m_device->CreateCommandQueue(&l_desc, MY_IID_PPV_ARGS(&m_queue));
	m_fenceEvent = CreateEvent(0, false, false, 0);
	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, MY_IID_PPV_ARGS(&m_fence));
}

Renderer::D3D12CmdQueue::~D3D12CmdQueue()
{
	
}

void Renderer::D3D12CmdQueue::Flush(uint64_t numLists, ID3D12CommandList* p_lists[],bool p_waitFinished)
{
	m_queue->ExecuteCommandLists(numLists, p_lists);
	m_queue->Signal(m_fence, m_fenceValue);
}

void Renderer::D3D12CmdQueue::WaitFinish()
{
	if (!IsFinished())
	{
		m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
		m_fenceValue++;
	}
}
