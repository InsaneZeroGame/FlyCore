#pragma once
#include <d3d12.h>
#include <array>
#include <queue>

namespace Renderer
{
	class D3D12CmdAllocatorPool: public Interface::INoCopy
	{
	public:
		static D3D12CmdAllocatorPool& GetPool()
		{
			static D3D12CmdAllocatorPool l_pool;
			return l_pool;
		};

		static D3D12CmdAllocatorPool* GetPoolPtr()
		{
			static D3D12CmdAllocatorPool l_pool;
			return &l_pool;
		};

		ID3D12CommandAllocator* RequestAllocator(D3D12_COMMAND_LIST_TYPE p_type);

		__forceinline void ReturnAllocator(ID3D12CommandAllocator* p_allocator, D3D12_COMMAND_LIST_TYPE p_type)
		{
			m_readyAllocators[p_type].push(p_allocator);
		}

		~D3D12CmdAllocatorPool();

	private:
		D3D12CmdAllocatorPool();

		ID3D12Device* m_device;

		//Direct Bundle Compute Copy
		std::array<std::queue<ID3D12CommandAllocator*>, 4> m_readyAllocators;
	};
}