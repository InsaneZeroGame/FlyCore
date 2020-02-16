#pragma once
#include "stdafx.h"
#include "D3D12Resource.h"



namespace Renderer
{
	class D3D12Buffer :  public D3D12Resource
	{
	public:
		
		virtual ~D3D12Buffer();

		virtual void CopyData(void* p_src ,uint64_t p_size)
		{
			if (!m_data) return;
			ASSERT(m_offset + p_size <= m_bufferSize,"Buffer Overflow");
			memcpy(m_data + m_offset, p_src, p_size);
			m_offset += p_size;
		}

		__forceinline uint64_t GetOffset() const
		{
			return m_offset;
		}

		__forceinline uint64_t GetBufferSize() const
		{
			return m_bufferSize;
		}

		virtual void ResetBuffer() {};

		virtual const D3D12Descriptor* GetSRV() 
		{
			return nullptr;
		};

	protected:
		D3D12Buffer(uint64_t p_size,
			D3D12_HEAP_TYPE p_heapType,
			D3D12_HEAP_FLAGS p_heapFlag,
			D3D12_RESOURCE_STATES p_initialState,
			D3D12_RESOURCE_DESC p_resourceDesc);

		ID3D12Device* m_device;

		uint8_t* m_data;

		uint64_t m_offset;
		//Max Buffer Size
		uint64_t m_bufferSize;


		virtual void CreateViews()
		{

		}

	};

	class D3D12VertexBuffer final : public D3D12Buffer
	{
	public:
		D3D12VertexBuffer(uint64_t p_size);
		~D3D12VertexBuffer();

		__forceinline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const
		{
			return m_vertexBufferView;
		}

		__forceinline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
		{
			return m_indexBufferView;
		}

	private:

		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;


	};


	using D3D12IndexBuffer = D3D12VertexBuffer;
	

	class D3D12UploadBuffer final : public D3D12Buffer
	{
	public:
		D3D12UploadBuffer(uint64_t p_size);
		~D3D12UploadBuffer();

		void CopyData(void* p_src, uint64_t p_size) override;

		__forceinline void ResetBuffer() override
		{
			m_offset = 0;
		}

		void CreateViews() override;

		const D3D12Descriptor* m_cbv;
	private:

	};
}