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
	
		void Flush(uint64_t numLists,ID3D12CommandList* p_lists[]);

		__forceinline bool IsFinished() const
		{
			return !(m_fence->GetCompletedValue() < m_fenceValue);
		}

		void WaitFinish();

	private:
		ID3D12Device* m_device = nullptr;

		ID3D12CommandQueue* m_queue = nullptr;
		
		uint64_t m_fenceValue;

		HANDLE m_fenceEvent;

		ID3D12Fence* m_fence;

	};
}