#pragma once
#include "stdafx.h"
#include "../Framework/IModule.h"
#include "../Framework/IRenderer.h"
#include "D3D12Device.h"
#include "D3D12CmdQueue.h"
#include "D3D12DescHeap.h"
#include "D3D12Texture.h"
#include "D3D12CmdManager.h"


namespace Renderer {

	class D3D12Renderer : public Interface::IRenderer, public Interface::IModule
	{
	public:
		D3D12Renderer();

		~D3D12Renderer();

		// Inherited via IModule
		virtual void OnInit() override;

		virtual void OnUpdate() override;

		virtual void OnDestory() override;

		// Inherited via IRenderer
		virtual void RenderScene() override;

	private:
		void InitCmdQueue();

		void InitCmdLists();

		void InitSwapChain();

		ID3D12DescriptorHeap* m_rtvHeap;

		UINT m_rtvDescriptorSize;

		D3D12CmdListManager* m_cmdListManager;

		ID3D12GraphicsCommandList* m_dummyCmdList;

		IDXGISwapChain1* m_swapChain1 = nullptr;

		ID3D12Device* m_device = nullptr;

		bool m_typedUAVLoadSupport_R11G11B10_FLOAT = false;
	
		bool m_typedUAVLoadSupport_R16G16B16A16_FLOAT = false;

		D3D12CmdQueue* m_cmdQueue = nullptr;

		std::array<D3D12Texture*,3> m_renderTargets;

	};
}