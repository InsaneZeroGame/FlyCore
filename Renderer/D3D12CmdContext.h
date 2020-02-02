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

		virtual void Begin(ID3D12PipelineState* p_state = nullptr);

		virtual void End(bool p_waitFinished);

	protected:
		D3D12CmdContext(D3D12_COMMAND_LIST_TYPE p_type);

		D3D12CmdQueue* m_cmdQueue;

		ID3D12CommandAllocator* m_cmdAllocator;

		virtual void WaitFinish(bool p_wait);
	};


	class D3D12GraphicsCmdContext final: public D3D12CmdContext
	{
	public:
		static D3D12GraphicsCmdContext& GetContext()
		{
			static D3D12GraphicsCmdContext l_context;
			return l_context;
		}

		void CopyBufferData(ID3D12Resource* pDstBuffer,
			UINT64 DstOffset,
			ID3D12Resource* pSrcBuffer,
			UINT64 SrcOffset,
			UINT64 NumBytes);

		void TransitResourceState(ID3D12Resource* pResource,
		D3D12_RESOURCE_STATES StateBefore,
		D3D12_RESOURCE_STATES StateAfter,
		UINT Subresource = 0);

		void Begin(ID3D12PipelineState* p_state) override;

		void End(bool p_waitFinished) override;

		void WaitFinish(bool p_wait) override;

		~D3D12GraphicsCmdContext();

	private:
		D3D12GraphicsCmdContext();

		ID3D12GraphicsCommandList* m_graphicsCmdList;


	};
}