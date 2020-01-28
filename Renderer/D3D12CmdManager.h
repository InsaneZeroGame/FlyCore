#pragma once
#include "stdafx.h"
#include "D3D12Device.h"



namespace Renderer
{
	class D3D12CmdListManager : Interface::INoCopy
	{
	public:
		static D3D12CmdListManager& GetManager()
		{
			D3D12CmdListManager l_manager;
			return l_manager;
		}

		static D3D12CmdListManager* GetManagerPtr()
		{
			static D3D12CmdListManager l_manager;
			return &l_manager;
		}

		~D3D12CmdListManager();

		void AllocateCmdList(D3D12_COMMAND_LIST_TYPE p_type, 
			ID3D12PipelineState* p_pipelineState,
			REFIID riid,
			_COM_Outptr_  void** ppCommandList);

		__forceinline void CloseCmdList(ID3D12GraphicsCommandList* p_cmdList)
		{
			p_cmdList->Close();
		};

		__forceinline void ResetCmdList(ID3D12GraphicsCommandList* p_cmdList,ID3D12PipelineState* pInitialState)
		{
			m_cmdAllocators[p_cmdList->GetType()]->Reset();
			p_cmdList->Reset(m_cmdAllocators[p_cmdList->GetType()], pInitialState);
		};

	private:

		//Direct Bundle Compute Copy = 4
		std::array<ID3D12CommandAllocator*, 4> m_cmdAllocators;

		D3D12CmdListManager();

		ID3D12Device* m_device;

	};
}