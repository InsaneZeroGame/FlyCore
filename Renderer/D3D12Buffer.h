#pragma once
#include "D3D12Resource.h"
#include "../Framework/Utility.h"
#include "../Gameplay/RenderComponent.h"

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

		__forceinline const D3D12Descriptor* GetSRV()
		{
			return m_srv;
		};

		__forceinline const D3D12Descriptor* GetUAV()
		{
			return m_uav;
		}

		__forceinline const D3D12Descriptor* GetCBV()
		{
			return m_cbv;
		}


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

		const D3D12Descriptor* m_cbv;

		const D3D12Descriptor* m_uav;
		
		const D3D12Descriptor* m_srv;

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
	

	class D3D12UploadBuffer : public D3D12Buffer
	{
	public:
		D3D12UploadBuffer(uint64_t p_size);
		virtual ~D3D12UploadBuffer();

		void CopyData(void* p_src, uint64_t p_size) override;

		__forceinline void UpdateData(uint64_t p_dst_offset, void* p_src, uint64_t p_size)
		{
			memcpy(m_data + p_dst_offset, p_src, p_size);
		};

		__forceinline void ResetBuffer() override
		{
			m_offset = 0;
		}

	private:
		void CreateViews() override;

	};

	class D3D12AnimBuffer final: public D3D12UploadBuffer
	{
	public:
		D3D12AnimBuffer(uint64_t p_actorCount):
			D3D12UploadBuffer(p_actorCount * sizeof(Gameplay::SkeletonAnim))
		{};
		~D3D12AnimBuffer()
		{};

		void UpdateActorAnim(uint64_t p_actorIndex,void* src);

		__forceinline D3D12_GPU_VIRTUAL_ADDRESS GetActorAnimBufferLocation(uint64_t p_actorIndex)
		{
			static int skeletionAnimSize = sizeof(Gameplay::SkeletonAnim);
			return m_GpuVirtualAddress + p_actorIndex * skeletionAnimSize;
		};

	private:

	};


	class D3D12StructBuffer final: public D3D12Buffer
	{
	public:
		D3D12StructBuffer(uint64_t p_elementCount,uint64_t p_elementSize);
		
		~D3D12StructBuffer();

	private:
		void CreateViews() override;

		uint64_t m_elementSize;

		uint64_t m_elementCount;
		
	};

	
}