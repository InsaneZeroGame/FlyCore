#pragma once
#include "D3D12DescManager.h"
#include "D3D12Texture.h"
#include <unordered_map>


namespace Renderer
{
	class D3D12GraphicsContext
	{
	public:

		static D3D12GraphicsContext& GetContext()
		{
			static D3D12GraphicsContext l_context;
			return l_context;
		}

		__forceinline void BeginRenderpass(std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> p_renderTargets, D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* p_depthBuffer, D3D12_RENDER_PASS_FLAGS p_flag = D3D12_RENDER_PASS_FLAG_NONE)
		{
			m_graphicsCmdList->BeginRenderPass(static_cast<UINT>(p_renderTargets.size()), p_renderTargets.data(), p_depthBuffer, p_flag);
		};

		__forceinline void BeginSwapchainOutputPass(const uint32_t& p_frameIndex)
		{
			m_graphicsCmdList->BeginRenderPass(1, &m_swapChainOutputPass.mrt[p_frameIndex], nullptr, D3D12_RENDER_PASS_FLAG_NONE);
		};

		__forceinline void EndRenderPass()
		{
			m_graphicsCmdList->EndRenderPass();
		};

		void TransitRenderTargets(std::vector<std::string>&& p_names, D3D12_RESOURCE_STATES p_stateBefore, D3D12_RESOURCE_STATES p_stateAfter);
		
		void TransitRenderTarget(const std::string& p_names, D3D12_RESOURCE_STATES p_stateBefore, D3D12_RESOURCE_STATES p_stateAfter);


		void BeginRender(const uint32_t& p_index);

		void EndRender();

		__forceinline void SetGraphicsCmdList(ID3D12GraphicsCommandList4* p_cmdList)
		{
			m_graphicsCmdList = p_cmdList;
		}

		__forceinline D3D12RenderTarget* GetRenderTarget(const std::string p_name)
		{
			return m_renderTargets[p_name];
		}

		__forceinline const D3D12Descriptor* GetDepthBuffer()
		{
			return m_depthBuffer->GetDSV();
		}

		void AddRenderTargets(const std::string& p_name,uint32_t p_width, uint32_t p_height, ID3D12Resource* p_resource, DXGI_FORMAT p_format);

		void InitSwapchainOutputTarget(uint32_t p_width,uint32_t p_height, IDXGISwapChain3* p_swapChain);

		~D3D12GraphicsContext();

	private:
		D3D12GraphicsContext();

		D3D12DepthBuffer* m_depthBuffer;

		std::array <D3D12RenderTarget*,3> m_swapChainOutputTargets;

		ID3D12Device* m_device;

		ID3D12GraphicsCommandList4* m_graphicsCmdList;

		uint32_t m_currentFrameIndex;

		uint32_t m_RTWidth;

		uint32_t m_RTHeight;

		D3D12_VIEWPORT m_viewPort;

		D3D12_RECT m_scissor;

		std::unordered_map<std::string, D3D12RenderTarget*> m_renderTargets;

		typedef struct RenderPass
		{
			std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> mrt;
			D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depth;
		}RenderPass;

		RenderPass m_swapChainOutputPass;
	};
}