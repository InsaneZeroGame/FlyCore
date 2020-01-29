#include "D3D12CmdManager.h"


Renderer::D3D12CmdListManager::D3D12CmdListManager()
{
	m_device = D3D12Device::GetDevice();
}

Renderer::D3D12CmdListManager::~D3D12CmdListManager()
{
}

void Renderer::D3D12CmdListManager::AllocateCmdList(
	D3D12_COMMAND_LIST_TYPE p_type, 
	ID3D12PipelineState* p_pipelineState,
	ID3D12CommandAllocator* p_cmdAllocator,
	REFIID riid,
	_COM_Outptr_  void** ppCommandList)
{
	switch (p_type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		m_device->CreateCommandList(0, p_type, p_cmdAllocator, p_pipelineState, riid, ppCommandList);
		break;
	case D3D12_COMMAND_LIST_TYPE_BUNDLE:
		break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		break;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		break;
	case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE:
		break;
	case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS:
		break;
	case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE:
		break;
	default:
		break;
	}
}
