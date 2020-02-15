#pragma once
#include "stdafx.h"
#include "D3D12Device.h"

namespace Renderer
{

	struct D3D12Descriptor
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		D3D12_DESCRIPTOR_HEAP_TYPE type;
	};


	class D3D12DescHeap : public Interface::INoCopy
	{
	public:
		D3D12DescHeap(D3D12_DESCRIPTOR_HEAP_TYPE p_type, uint32_t  p_descNum = 256,D3D12_DESCRIPTOR_HEAP_FLAGS p_flag = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		
		~D3D12DescHeap();

		__forceinline ID3D12DescriptorHeap* GetHeap() { return m_descHeap; }

		const D3D12Descriptor* RequestDesc();

		__forceinline void ReturnDesc(const D3D12Descriptor*  p_desc)
		{
			m_descs.push(p_desc);
		}

	private:
		ID3D12Device* m_device = nullptr;

		ID3D12DescriptorHeap* m_descHeap = nullptr;

		uint64_t m_descHandleIncrementSize;

		std::queue<const D3D12Descriptor*> m_descs;

		const uint64_t m_maxDescCount;

		uint64_t m_descOffset;

		uint64_t m_allocatedCount;

		const D3D12_DESCRIPTOR_HEAP_TYPE m_type;
	};
}