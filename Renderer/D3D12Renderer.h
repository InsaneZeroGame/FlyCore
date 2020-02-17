#pragma once
#include "stdafx.h"
#include "../Framework/IModule.h"
#include "../Framework/IRenderer.h"
#include "D3D12Device.h"
#include "D3D12CmdQueue.h"
#include "D3D12DescHeap.h"
#include "D3D12Texture.h"
#include "D3D12CmdManager.h"
#include "D3D12CmdAllocatorPool.h"
#include "D3D12Buffer.h"
#include "D3D12CmdContext.h"
#include "D3D12GraphicsContext.h"


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
		void RenderScene() override;

		void LoadScene(Renderer::Scene*) override;


	private:
		void InitCmdQueue();

		void InitCmdLists();

		void InitSwapChain();

		void InitSyncPrimitive();

		void InitGraphicsContext();

		void SyncFrame();

		void InitBuffers();

		void InitRootSignature();


		//Default texture
		void CreateDefaultTexture();



		Renderer::Scene* m_scene;


		ID3D12RootSignature* m_rootSignature;

		void InitPipelineState();

		ID3D12PipelineState* m_pipelineState;

		D3D12VertexBuffer* m_vertexBuffer;

		D3D12IndexBuffer* m_indexBuffer;

		D3D12UploadBuffer* m_uploadBuffer;

		D3D12UploadBuffer* m_cameraUniformBuffer;

		HANDLE m_fenceEvent;

		std::array<uint64_t, 3> m_fenceValues;

		ID3D12Fence* m_fence;

		D3D12CmdListManager* m_cmdListManager;

		D3D12CmdAllocatorPool* m_cmdAllocatorPool;

		ID3D12GraphicsCommandList* m_graphicsCmdList;

		std::array<ID3D12CommandAllocator*, Constants::SWAPCHAIN_BUFFER_COUNT> m_graphicsCmdAllocator;

		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain = nullptr;

		ID3D12Device* m_device = nullptr;

		bool m_typedUAVLoadSupport_R11G11B10_FLOAT = false;
	
		bool m_typedUAVLoadSupport_R16G16B16A16_FLOAT = false;

		D3D12CmdQueue* m_cmdQueue = nullptr;

		uint32_t m_frameIndex;
	};
}