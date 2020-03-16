#include "stdafx.h"

#include "D3D12CmdContext.h"
#include "D3D12Device.h"

Renderer::D3D12CmdContext::D3D12CmdContext(D3D12_COMMAND_LIST_TYPE p_type):
	m_cmdQueue(nullptr),
	m_cmdAllocator(nullptr)
{
	m_cmdQueue = new D3D12CmdQueue(p_type);
	m_cmdAllocator = D3D12CmdAllocatorPool::GetPool().RequestAllocator(p_type);
}


void Renderer::D3D12CmdContext::WaitFinish(bool p_wait)
{

}

Renderer::D3D12CmdContext::~D3D12CmdContext()
{
	SAFE_DELETE(m_cmdQueue);
}

void Renderer::D3D12CmdContext::Begin(ID3D12PipelineState* p_state)
{
}

void Renderer::D3D12CmdContext::End(bool p_waitFinished)
{
}

Renderer::D3D12GraphicsCmdContext::D3D12GraphicsCmdContext():
	D3D12CmdContext(D3D12_COMMAND_LIST_TYPE_DIRECT),
	m_graphicsCmdList(nullptr)
{
	D3D12CmdListManager::GetManager().AllocateCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT,
		nullptr,
		m_cmdAllocator,
		MY_IID_PPV_ARGS(&m_graphicsCmdList));
	m_graphicsCmdList->Close();
}

void Renderer::D3D12GraphicsCmdContext::WaitFinish(bool p_wait)
{
	
}

void Renderer::D3D12GraphicsCmdContext::UploadVertexBuffer(D3D12VertexBuffer* p_dst,
	uint64_t p_dstOffset,
	D3D12UploadBuffer* p_src,
	uint64_t p_srcOffset,
	uint64_t p_size)
{
	CopyBufferData(p_dst->GetResource(),
		p_dstOffset,
		p_src->GetResource(),
		p_srcOffset,
		p_size);
}

void Renderer::D3D12GraphicsCmdContext::CopyBufferData(ID3D12Resource* pDstBuffer,
	UINT64 DstOffset,
	ID3D12Resource* pSrcBuffer,
	UINT64 SrcOffset,
	UINT64 NumBytes)
{
	m_graphicsCmdList->CopyBufferRegion(pDstBuffer, DstOffset, pSrcBuffer,SrcOffset, NumBytes);
}

void Renderer::D3D12GraphicsCmdContext::CopyTextureData(
	ID3D12Resource* pDstBuffer, 
	ID3D12Resource* pSrcBuffers, 
	uint32_t p_width,
	uint32_t p_height,
	DXGI_FORMAT p_format)
{
	D3D12_TEXTURE_COPY_LOCATION l_destLocation = {};
	l_destLocation.pResource = pDstBuffer;
	l_destLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	l_destLocation.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION l_srcLocation = {};
	l_srcLocation.pResource = pSrcBuffers;
	l_srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	auto texelInByte = GetFormatInByte(p_format);
	l_srcLocation.PlacedFootprint.Footprint = { p_format ,p_width,p_height,1,static_cast<uint32_t>(Utility::AlignTo256(p_width  * texelInByte))};
	l_srcLocation.PlacedFootprint.Offset = 0;

	D3D12_BOX l_box = {};
	l_box.left = 0;
	l_box.top = 0;
	l_box.front = 0;
	l_box.right = p_width;
	l_box.bottom = p_height;
	l_box.back = 1;
	m_graphicsCmdList->CopyTextureRegion(&l_destLocation, 0, 0, 0, &l_srcLocation, &l_box);
}

void Renderer::D3D12GraphicsCmdContext::TransitResourceState(ID3D12Resource* pResource,
	D3D12_RESOURCE_STATES StateBefore,
	D3D12_RESOURCE_STATES StateAfter,
	UINT Subresource)
{
	D3D12_RESOURCE_BARRIER l_barrier = {};
	l_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	l_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	l_barrier.Transition = { pResource,Subresource,StateBefore,StateAfter};
	m_graphicsCmdList->ResourceBarrier(1, &l_barrier);
}

void Renderer::D3D12GraphicsCmdContext::Begin(ID3D12PipelineState* p_state)
{
	m_cmdAllocator->Reset();
	m_graphicsCmdList->Reset(m_cmdAllocator, p_state);
}

void Renderer::D3D12GraphicsCmdContext::End(bool p_waitFinished)
{
	m_graphicsCmdList->Close();
	ID3D12CommandList* l_cmdLists[] = { m_graphicsCmdList };
	m_cmdQueue->Flush(1, l_cmdLists);
	if (p_waitFinished)
	{
		m_cmdQueue->WaitFinish();
	}
}

Renderer::D3D12GraphicsCmdContext::~D3D12GraphicsCmdContext()
{
}
