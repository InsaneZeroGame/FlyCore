#include "D3D12Resource.h"
#include "D3D12Buffer.h"
#include "D3D12Device.h"


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

}

Renderer::D3D12Buffer::~D3D12Buffer()
{
}


Renderer::D3D12VertexBuffer::D3D12VertexBuffer(uint64_t p_size):
	D3D12Buffer(p_size,D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS,
		D3D12_RESOURCE_STATE_COPY_DEST,
		{ D3D12_RESOURCE_DIMENSION_BUFFER ,0,p_size,1,1,1,DXGI_FORMAT_UNKNOWN,{1,0},D3D12_TEXTURE_LAYOUT_ROW_MAJOR,D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE })
{
}

Renderer::D3D12VertexBuffer::~D3D12VertexBuffer()
{
}

Renderer::D3D12UploadBuffer::D3D12UploadBuffer(uint64_t p_size):
	D3D12Buffer(p_size, D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		{ D3D12_RESOURCE_DIMENSION_BUFFER ,0,p_size,1,1,1,DXGI_FORMAT_UNKNOWN,{1,0},D3D12_TEXTURE_LAYOUT_ROW_MAJOR,D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE })

{
	m_data = new uint8_t[m_bufferSize];
}

Renderer::D3D12UploadBuffer::~D3D12UploadBuffer()
{
	if (m_data)
	{
		delete[] m_data;
		m_data = nullptr;
	}
}
