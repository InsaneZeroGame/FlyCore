#pragma once
#include "stdafx.h"
#include "D3D12Device.h"

namespace Renderer
{
	class D3D12DescHeap : public Interface::INoCopy
	{
	public:
		D3D12DescHeap(D3D12_DESCRIPTOR_HEAP_TYPE p_type, uint32_t  p_descNum = 256,D3D12_DESCRIPTOR_HEAP_FLAGS p_flag = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		
		~D3D12DescHeap();

		__forceinline ID3D12DescriptorHeap* GetHeap() { return m_descHeap; }

	private:
		ID3D12Device* m_device = nullptr;

		ID3D12DescriptorHeap* m_descHeap = nullptr;
	};
}