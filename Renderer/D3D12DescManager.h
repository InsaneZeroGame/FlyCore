#pragma once
#include "D3D12DescHeap.h"


namespace Renderer
{
	class D3D12DescManager : public Interface::INoCopy
	{
	public:
		static D3D12DescManager& GetDescManager()
		{
			static D3D12DescManager l_descManager;
			return l_descManager;
		}

		__forceinline D3D12Descriptor* RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE p_type)
		{
			return m_descHeaps[p_type]->RequestDesc();
		}

		__forceinline void ReturnDesc(D3D12Descriptor* p_desc)
		{
			m_descHeaps[p_desc->type]->ReturnDesc(p_desc);
		}

		__forceinline D3D12DescHeap* GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE p_type) { 
			return m_descHeaps[p_type]; 
		}


		~D3D12DescManager();

	private:
		D3D12DescManager();

		D3D12DescHeap* m_descHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		uint64_t m_descIncrementSize[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	};
}