#pragma once
#include "stdafx.h"
#include "D3D12CmdQueue.h"
#include "D3D12CmdManager.h"
#include "D3D12CmdAllocatorPool.h"

namespace Renderer
{
	class D3D12CmdContext : public Interface::INoCopy
	{
	public:

		virtual ~D3D12CmdContext();


	protected:
		D3D12CmdContext(D3D12_COMMAND_LIST_TYPE p_type);

		D3D12CmdQueue* m_cmdQueue;

		ID3D12CommandAllocator* m_cmdAllocator;

		void WaitFinish();
	};


	class D3D12GraphicsCmdContext final: public D3D12CmdContext
	{
	public:
		static D3D12GraphicsCmdContext& GetContext()
		{
			static D3D12GraphicsCmdContext l_context;
			return l_context;
		}

		~D3D12GraphicsCmdContext();

	private:
		D3D12GraphicsCmdContext();

		ID3D12GraphicsCommandList* m_graphicsCmdList;

	};
}