#include "D3D12GraphicsContext.h"


Renderer::D3D12GraphicsContext::D3D12GraphicsContext():
	m_device(D3D12Device::GetDevice()),
	m_graphicsCmdList(nullptr),
	m_currentFrameIndex(0),
	m_RTWidth(0),
	m_RTHeight(0)
{
}

void Renderer::D3D12GraphicsContext::BeginRender(const uint32_t& p_index)
{
	m_currentFrameIndex = p_index;
	m_graphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[p_index]->GetResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_graphicsCmdList->OMSetRenderTargets(1, &m_renderTargets[m_currentFrameIndex]->GetRTV()->cpuHandle,false, &m_depthBuffer->GetDSV()->cpuHandle);
	m_graphicsCmdList->RSSetViewports(1, &m_viewPort);
	m_graphicsCmdList->RSSetScissorRects(1, &m_scissor);
	m_graphicsCmdList->ClearRenderTargetView(m_renderTargets[m_currentFrameIndex]->GetRTV()->cpuHandle, Constants::CLEAR_COLOR, 0, nullptr);
	m_graphicsCmdList->ClearDepthStencilView(m_depthBuffer->GetDSV()->cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

}

void Renderer::D3D12GraphicsContext::EndRender()
{
	m_graphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currentFrameIndex]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

}

void Renderer::D3D12GraphicsContext::InitRenderTargets(uint32_t p_width, uint32_t p_height, IDXGISwapChain3* p_swapChain)
{
	m_RTHeight = p_height;
	m_RTWidth = p_width;

	m_viewPort = { 0.0f,0.0f,static_cast<float>(m_RTWidth),static_cast<float>(m_RTHeight),0.0f,1.0f };
	m_scissor = { 0,0,m_RTWidth,m_RTHeight };

	for (uint32_t i = 0; i < Constants::SWAPCHAIN_BUFFER_COUNT; ++i)
	{
		ID3D12Resource* DisplayPlane;
		ASSERT_SUCCEEDED(p_swapChain->GetBuffer(i, MY_IID_PPV_ARGS(&DisplayPlane)));
		m_renderTargets[i] = new D3D12RenderTarget(p_width,p_height, DisplayPlane);
	}

	//Create depth buffer resource
	m_depthBuffer = new D3D12DepthBuffer(p_width, p_width);

}

Renderer::D3D12GraphicsContext::~D3D12GraphicsContext()
{
	for (auto i = 0; i < m_renderTargets.size(); ++i)
	{
		SAFE_DELETE(m_renderTargets[i]);
	}
}
