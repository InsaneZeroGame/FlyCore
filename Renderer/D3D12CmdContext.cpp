#include "D3D12CmdContext.h"
#include "D3D12Device.h"

Renderer::D3D12CmdContext::D3D12CmdContext(D3D12_COMMAND_LIST_TYPE p_type):
	m_cmdQueue(nullptr),
	m_cmdAllocator(nullptr)
{
	m_cmdQueue = new D3D12CmdQueue(p_type);
	m_cmdAllocator = D3D12CmdAllocatorPool::GetPool().RequestAllocator(p_type);
}


void Renderer::D3D12CmdContext::WaitFinish()
{
}

Renderer::D3D12CmdContext::~D3D12CmdContext()
{
	SAFE_DELETE(m_cmdQueue);
}

Renderer::D3D12GraphicsCmdContext::D3D12GraphicsCmdContext():
	D3D12CmdContext(D3D12_COMMAND_LIST_TYPE_DIRECT),
	m_graphicsCmdList(nullptr)
{
	D3D12CmdListManager::GetManager().AllocateCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT,
		nullptr,
		m_cmdAllocator,
		MY_IID_PPV_ARGS(&m_graphicsCmdList));
	m_graphicsCmdList->Close();
}

Renderer::D3D12GraphicsCmdContext::~D3D12GraphicsCmdContext()
{
}
