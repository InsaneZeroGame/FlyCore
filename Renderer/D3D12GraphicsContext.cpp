#include "stdafx.h"
#include "D3D12GraphicsContext.h"
#include "D3D12CmdContext.h"

Renderer::D3D12GraphicsContext::D3D12GraphicsContext():
	m_device(D3D12Device::GetDevice()),
	m_graphicsCmdList(nullptr),
	m_currentFrameIndex(0),
	m_RTWidth(0),
	m_RTHeight(0)
{
}

void Renderer::D3D12GraphicsContext::TransitRenderTargets(std::vector<std::string>&& p_names, D3D12_RESOURCE_STATES p_stateBefore, D3D12_RESOURCE_STATES p_stateAfter)
{
	std::vector<D3D12_RESOURCE_BARRIER> l_barriers;
	l_barriers.resize(p_names.size());

	for (auto i = 0; i < p_names.size(); ++i)
	{
		l_barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		l_barriers[i].Transition = { m_renderTargets[p_names[i]]->GetResource(), 0,p_stateBefore,p_stateAfter };
	}
	m_graphicsCmdList->ResourceBarrier(static_cast<uint32_t>(l_barriers.size()),l_barriers.data());

}

void Renderer::D3D12GraphicsContext::TransitRenderTarget(const std::string& p_name, D3D12_RESOURCE_STATES p_stateBefore, D3D12_RESOURCE_STATES p_stateAfter)
{
	D3D12_RESOURCE_BARRIER l_barrier;
	l_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	l_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	l_barrier.Transition = { m_renderTargets[p_name]->GetResource(), 0,p_stateBefore,p_stateAfter };
	m_graphicsCmdList->ResourceBarrier(1, &l_barrier);
}

void Renderer::D3D12GraphicsContext::BeginRender(const uint32_t& p_index)
{
	m_currentFrameIndex = p_index;
	m_graphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swapChainOutputTargets[p_index]->GetResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_graphicsCmdList->RSSetViewports(1, &m_viewPort);
	m_graphicsCmdList->RSSetScissorRects(1, &m_scissor);
}

void Renderer::D3D12GraphicsContext::EndRender()
{
	m_graphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_swapChainOutputTargets[m_currentFrameIndex]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}

void Renderer::D3D12GraphicsContext::AddRenderTargets(const std::string& p_name,uint32_t p_width, uint32_t p_height, ID3D12Resource* p_resource, DXGI_FORMAT p_format)
{
	D3D12RenderTarget* l_newRenderTarget = new D3D12RenderTarget(p_width, p_height, nullptr, p_format);
	l_newRenderTarget->SetName(MakeWStr(p_name));
	m_renderTargets.insert(std::pair<std::string, D3D12RenderTarget*>(p_name, l_newRenderTarget));
	
}

void Renderer::D3D12GraphicsContext::InitSwapchainOutputTarget(uint32_t p_width, uint32_t p_height, IDXGISwapChain3* p_swapChain)
{
	m_RTHeight = p_height;
	m_RTWidth = p_width;

	m_viewPort = { 0.0f,0.0f,static_cast<float>(m_RTWidth),static_cast<float>(m_RTHeight),0.0f,1.0f };
	m_scissor = { 0,0,static_cast<LONG>(m_RTWidth),static_cast<LONG>(m_RTHeight) };

	m_swapChainOutputPass.mrt.resize(Constants::SWAPCHAIN_BUFFER_COUNT);


	for (uint32_t i = 0; i < Constants::SWAPCHAIN_BUFFER_COUNT; ++i)
	{
		ID3D12Resource* DisplayPlane;
		ASSERT_SUCCEEDED(p_swapChain->GetBuffer(i, MY_IID_PPV_ARGS(&DisplayPlane)));
		m_swapChainOutputTargets[i] = new D3D12RenderTarget(p_width,p_height, DisplayPlane,Constants::SwapChainFormat);
		
		m_swapChainOutputPass.mrt[i].BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
		m_swapChainOutputPass.mrt[i].BeginningAccess.Clear.ClearValue.Color[0] = Constants::CLEAR_COLOR[0];
		m_swapChainOutputPass.mrt[i].BeginningAccess.Clear.ClearValue.Color[1] = Constants::CLEAR_COLOR[1];
		m_swapChainOutputPass.mrt[i].BeginningAccess.Clear.ClearValue.Color[2] = Constants::CLEAR_COLOR[2];
		m_swapChainOutputPass.mrt[i].BeginningAccess.Clear.ClearValue.Color[3] = Constants::CLEAR_COLOR[3];
		m_swapChainOutputPass.mrt[i].BeginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
		m_swapChainOutputPass.mrt[i].cpuDescriptor = m_swapChainOutputTargets[i]->GetRTV()->cpuHandle;
		m_swapChainOutputPass.mrt[i].EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
	}

	//Create depth buffer resource
	m_depthBuffer = new D3D12DepthBuffer(p_width, p_height);
	m_depthBuffer->SetName(L"Depth Buffer");

	m_shadowMap = new D3D12DepthBuffer(ShadowMapWidth, ShadowMapHeight);
	m_shadowMap->SetName(L"Shadow Map");
}

Renderer::D3D12GraphicsContext::~D3D12GraphicsContext()
{
	for (auto i = 0; i < m_swapChainOutputTargets.size(); ++i)
	{
		SAFE_DELETE(m_swapChainOutputTargets[i]);
	}
	for (auto i = m_renderTargets.begin(); i != m_renderTargets.end(); ++i)
	{
		SAFE_DELETE(i->second);
	}

}
