#pragma once
#include "stdafx.h"

namespace Renderer
{
	class D3D12CmdQueue
	{
	public:
		D3D12CmdQueue(D3D12_COMMAND_LIST_TYPE);
		~D3D12CmdQueue();
		ID3D12CommandQueue* GetQueue() const { return m_queue; }
	private:
		ID3D12Device* m_device = nullptr;
		ID3D12CommandQueue* m_queue = nullptr;
	};
}