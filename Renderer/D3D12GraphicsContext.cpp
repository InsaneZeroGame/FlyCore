#include "D3D12GraphicsContext.h"

Renderer::D3D12GraphicsContext::D3D12GraphicsContext():
	m_device(D3D12Device::GetDevice()),
	m_graphicsCmdList(nullptr),
	m_currentFrameIndex(0)
{
}

void Renderer::D3D12GraphicsContext::BeginRender(const uint32_t& p_index)
{
	m_currentFrameIndex = p_index;
	m_graphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTextures[p_index]->GetResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_graphicsCmdList->OMSetRenderTargets(1, &m_renderTargetsDesc[m_currentFrameIndex]->cpuHandle, false, nullptr);
	m_graphicsCmdList->ClearRenderTargetView(m_renderTargetsDesc[m_currentFrameIndex]->cpuHandle, Constants::CLEAR_COLOR, 0, nullptr);
}

void Renderer::D3D12GraphicsContext::EndRender()
{
	m_graphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTextures[m_currentFrameIndex]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

}

void Renderer::D3D12GraphicsContext::InitRenderTargets(uint32_t p_width, uint32_t p_height, IDXGISwapChain3* p_swapChain)
{
	//Request 3 descs for render targets.
	for (auto i = 0; i < m_renderTargetsDesc.size(); i++)
	{
		m_renderTargetsDesc[i] = D3D12DescManager::GetDescManager().RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	D3D12_RESOURCE_DESC l_swapChainDesc = {};
	l_swapChainDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	l_swapChainDesc.Alignment = 0;
	l_swapChainDesc.Width = p_width;
	l_swapChainDesc.Height = p_height;
	l_swapChainDesc.DepthOrArraySize = 1;
	l_swapChainDesc.MipLevels = 0;
	l_swapChainDesc.Format = Constants::SwapChainFormat;
	l_swapChainDesc.SampleDesc.Count = 1;
	l_swapChainDesc.SampleDesc.Quality = 0;
	l_swapChainDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	l_swapChainDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	for (uint32_t i = 0; i < Constants::SWAPCHAIN_BUFFER_COUNT; ++i)
	{
		ID3D12Resource* DisplayPlane;
		ASSERT_SUCCEEDED(p_swapChain->GetBuffer(i, MY_IID_PPV_ARGS(&DisplayPlane)));
		m_renderTextures[i] = new D3D12Texture(&l_swapChainDesc, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES, DisplayPlane);
		m_device->CreateRenderTargetView(m_renderTextures[i]->GetResource(), nullptr, m_renderTargetsDesc[i]->cpuHandle);
	}
}

Renderer::D3D12GraphicsContext::~D3D12GraphicsContext()
{
	for (auto i = 0; i < m_renderTextures.size(); ++i)
	{
		SAFE_DELETE(m_renderTextures[i]);
	}
}
