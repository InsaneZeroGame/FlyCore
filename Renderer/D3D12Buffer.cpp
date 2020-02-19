#include "D3D12Resource.h"
#include "D3D12Buffer.h"
#include "D3D12Device.h"
#include "../Framework/IScene.h"
#include "D3D12DescManager.h"


Renderer::D3D12Buffer::D3D12Buffer(uint64_t p_size,
	D3D12_HEAP_TYPE p_heapType, 
	D3D12_HEAP_FLAGS p_heapFlag, 
	D3D12_RESOURCE_STATES p_initialState,
	D3D12_RESOURCE_DESC p_resourceDesc):
	D3D12Resource(),
	m_bufferSize(p_size),
	m_offset(0),
	m_device(D3D12Device::GetDevice())
{
	D3D12_HEAP_PROPERTIES l_heapProperties = {};
	l_heapProperties.Type = p_heapType;
	l_heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	l_heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	l_heapProperties.CreationNodeMask = 0;
	l_heapProperties.VisibleNodeMask = 0;
	m_UsageState = p_initialState;
	m_device->CreateCommittedResource(&l_heapProperties,
		p_heapFlag,
		&p_resourceDesc,
		m_UsageState,
		nullptr,
		MY_IID_PPV_ARGS(&m_pResource));
	m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();

}

Renderer::D3D12Buffer::~D3D12Buffer()
{
}


Renderer::D3D12VertexBuffer::D3D12VertexBuffer(uint64_t p_size):
	D3D12Buffer(p_size,D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		D3D12_RESOURCE_STATE_COPY_DEST,
		{ D3D12_RESOURCE_DIMENSION_BUFFER ,0,p_size,1,1,1,DXGI_FORMAT_UNKNOWN,{1,0},D3D12_TEXTURE_LAYOUT_ROW_MAJOR,D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE })
{
	m_vertexBufferView.BufferLocation = m_GpuVirtualAddress;
	m_vertexBufferView.SizeInBytes = (UINT)p_size;
	m_vertexBufferView.StrideInBytes = sizeof(Renderer::Vertex);

	m_indexBufferView.BufferLocation = m_GpuVirtualAddress;
	m_indexBufferView.SizeInBytes = (UINT)p_size;
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

Renderer::D3D12VertexBuffer::~D3D12VertexBuffer()
{
}

Renderer::D3D12UploadBuffer::D3D12UploadBuffer(uint64_t p_size):
	D3D12Buffer(p_size, D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		{ D3D12_RESOURCE_DIMENSION_BUFFER ,0,p_size,1,1,1,DXGI_FORMAT_UNKNOWN,{1,0},D3D12_TEXTURE_LAYOUT_ROW_MAJOR,D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE })

{
	//Map Here and keep buffer mapped
	D3D12_RANGE l_mapRange = {};
	l_mapRange.Begin = 0;
	l_mapRange.End = p_size;
	void* l_dataPtr;
	m_pResource->Map(0, &l_mapRange, &l_dataPtr);
	m_data = (uint8_t*)l_dataPtr;

	CreateViews();
}

Renderer::D3D12UploadBuffer::~D3D12UploadBuffer()
{
	D3D12_RANGE l_mapRange = {};
	l_mapRange.Begin = 0;
	l_mapRange.End = m_bufferSize;
	void* l_dataPtr = m_data;
	m_pResource->Unmap(0, &l_mapRange);
}

void Renderer::D3D12UploadBuffer::CopyData(void* p_src, uint64_t p_size)
{
	memcpy(m_data + m_offset, p_src, p_size);
	m_offset += p_size;
}

void Renderer::D3D12UploadBuffer::CreateViews()
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC l_cbvDesc = {};
	l_cbvDesc.BufferLocation = m_GpuVirtualAddress;
	l_cbvDesc.SizeInBytes = UINT(m_bufferSize);
	m_cbv = D3D12DescManager::GetDescManager().RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_device->CreateConstantBufferView(&l_cbvDesc, m_cbv->cpuHandle);
}



Renderer::D3D12StructBuffer::D3D12StructBuffer(uint64_t p_size):
	D3D12Buffer(p_size,)
{
}

Renderer::D3D12StructBuffer::~D3D12StructBuffer()
{
}
