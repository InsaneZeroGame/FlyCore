#include "D3D12CmdQueue.h"
#include "D3D12Device.h"


Renderer::D3D12CmdQueue::D3D12CmdQueue(D3D12_COMMAND_LIST_TYPE p_type):
	m_device(D3D12Device::GetDevice())
{
	D3D12_COMMAND_QUEUE_DESC l_desc = {};
	l_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	l_desc.NodeMask = 0;
	l_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	l_desc.Type = p_type;
	m_device->CreateCommandQueue(&l_desc, MY_IID_PPV_ARGS(&m_queue));
}

Renderer::D3D12CmdQueue::~D3D12CmdQueue()
{
}
