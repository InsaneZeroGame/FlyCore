#pragma once
#include "stdafx.h"
#include "D3D12DescManager.h"
#include "D3D12Texture.h"

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

		void BeginRender(const uint32_t& p_index);

		void EndRender();

		__forceinline void SetGraphicsCmdList(ID3D12GraphicsCommandList* p_cmdList)
		{
			m_graphicsCmdList = p_cmdList;
		}

		void InitRenderTargets(uint32_t p_width,uint32_t p_height, IDXGISwapChain3* p_swapChain);

		~D3D12GraphicsContext();

	private:
		D3D12GraphicsContext();

		D3D12DepthBuffer* m_depthBuffer;

		std::array <D3D12RenderTarget*,3> m_renderTargets;

		ID3D12Device* m_device;

		ID3D12GraphicsCommandList* m_graphicsCmdList;

		uint32_t m_currentFrameIndex;

		uint32_t m_RTWidth;

		uint32_t m_RTHeight;

	};
}