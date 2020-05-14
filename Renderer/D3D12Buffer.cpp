#include "stdafx.h"
#include "D3D12Resource.h"
#include "D3D12Buffer.h"
#include "D3D12Device.h"
#include "D3D12DescManager.h"
#include "../Gameplay/RenderComponent.h"

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
	m_vertexBufferView.StrideInBytes = sizeof(Gameplay::Vertex);

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
	//if (!m_pResource) return;
	//D3D12_RANGE l_mapRange = {};
	//l_mapRange.Begin = 0;
	//l_mapRange.End = m_bufferSize;
	//void* l_dataPtr = m_data;
	//m_pResource->Unmap(0, &l_mapRange);
}

void Renderer::D3D12UploadBuffer::CopyData(void* p_src, uint64_t p_size)
{
	assert(p_size <= m_bufferSize);
	memcpy(m_data + m_offset, p_src, p_size);
	m_offset += p_size;
}


void Renderer::D3D12UploadBuffer::CreateViews()
{
	//D3D12_CONSTANT_BUFFER_VIEW_DESC l_cbvDesc = {};
	//l_cbvDesc.BufferLocation = m_GpuVirtualAddress;
	//l_cbvDesc.SizeInBytes = UINT(m_bufferSize);
	//m_cbv = D3D12DescManager::GetDescManager().RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//m_device->CreateConstantBufferView(&l_cbvDesc, m_cbv->cpuHandle);
}



Renderer::D3D12StructBuffer::D3D12StructBuffer(uint64_t p_elementCount, uint64_t p_elementSize):
	D3D12Buffer(static_cast<uint64_t>(p_elementCount * p_elementSize),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE,
		D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		{
			D3D12_RESOURCE_DIMENSION_BUFFER,
			0,
			p_elementCount* p_elementSize,
			1,
			1,
			1,
			DXGI_FORMAT::DXGI_FORMAT_UNKNOWN,
		{1,0},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
		}),
	m_elementCount(p_elementCount),
	m_elementSize(p_elementSize)
{
	CreateViews();
}

Renderer::D3D12StructBuffer::~D3D12StructBuffer()
{
}

void Renderer::D3D12StructBuffer::CreateViews()
{

	m_uav = D3D12DescManager::GetDescManager().RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	//Create UAV
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC l_desc = {};
		l_desc.Format = DXGI_FORMAT_UNKNOWN;
		l_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		l_desc.Buffer.FirstElement = 0;
		l_desc.Buffer.NumElements = static_cast<uint32_t>(m_elementCount);
		l_desc.Buffer.StructureByteStride = static_cast<uint32_t>(m_elementSize);
		l_desc.Buffer.CounterOffsetInBytes = 0;
		l_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		m_device->CreateUnorderedAccessView(m_pResource.Get(), nullptr, &l_desc, m_uav->cpuHandle);
	}

	m_srv = D3D12DescManager::GetDescManager().RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//Create SRV
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC l_desc = {};
		l_desc.Format = DXGI_FORMAT_UNKNOWN;
		l_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		l_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		l_desc.Buffer.FirstElement = 0;
		l_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		l_desc.Buffer.NumElements = static_cast<uint32_t>(m_elementCount);
		l_desc.Buffer.StructureByteStride = static_cast<uint32_t>(m_elementSize);
		m_device->CreateShaderResourceView(m_pResource.Get(), &l_desc, m_srv->cpuHandle);
	}
}

void Renderer::D3D12AnimBuffer::UpdateActorAnim(uint64_t p_actorIndex, void* src)
{
	static int skeletionAnimSize = sizeof(Gameplay::SkeletonAnim);
	UpdateData(p_actorIndex * skeletionAnimSize, src, skeletionAnimSize);
}
