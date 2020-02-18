#pragma once
#include "stdafx.h"
#include "D3D12CmdQueue.h"
#include "D3D12CmdAllocatorPool.h"
#include "D3D12CmdManager.h"

namespace Renderer
{
	class D3D12Cmd : public Interface::INoCopy
	{
	public:
		virtual ~D3D12Cmd();

		virtual void Flush() const {};

		static ID3D12CommandQueue* GetQueue() { 
			return m_cmdQueue->GetQueue(); 
		}

	protected:
		D3D12CmdListManager* m_cmdListManager;

		D3D12Cmd(D3D12_COMMAND_LIST_TYPE p_type,uint32_t p_cmdListCount);

		D3D12CmdAllocatorPool* m_cmdAllocatorPool;

		static D3D12CmdQueue* m_cmdQueue;

		std::vector<ID3D12CommandAllocator*> m_cmdAllocator;

	};

	class D3D12GraphicsCmd final: public D3D12Cmd
	{
	public:
		D3D12GraphicsCmd(const uint32_t& p_count);

		~D3D12GraphicsCmd();
		
		__forceinline void Close() { m_cmdList->Close(); }
		
		void Flush() const override;

		void Reset(const uint32_t& p_index,ID3D12PipelineState* p_state = nullptr) const;

		__forceinline ID3D12GraphicsCommandList* GetCmd()
		{
			return m_cmdList;
		}

		operator ID3D12GraphicsCommandList* () { return m_cmdList; }
	private:
		ID3D12GraphicsCommandList* m_cmdList;
	};

	
}