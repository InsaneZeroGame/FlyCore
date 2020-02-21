#pragma once
#include "../Framework/IModule.h"
#include "../Framework/IRenderer.h"
#include "D3D12Device.h"
#include "D3D12DescHeap.h"
#include "D3D12Texture.h"
#include "D3D12Buffer.h"
#include "D3D12CmdContext.h"
#include "D3D12GraphicsContext.h"
#include "D3D12RenderCmd.h"
#include "../3dparty/include/glm/glm.hpp"
#include "../3dparty/include/glm/ext.hpp"

namespace Renderer {

	struct PointLight
	{
		float position[4];
		float color[4];
		float attenutation;
		uint32_t isActive;

		PointLight() : 
			isActive(0)
		{
			
		}
	};

	struct LightList
	{
		//Todo use bit mask to
		//reduce memory cost
		uint32_t isActive[256];
	};

	struct SceneUniformBuffer
	{
		glm::mat4x4 m_proj;
		glm::mat4x4 m_view;
		glm::mat4x4 m_inverProj;
		float zNear;
		float zFar;
		std::array<PointLight, 256> m_lights;
	};


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
		void InitSwapChain();

		void InitSyncPrimitive();

		void InitGraphicsContext();

		void SyncFrame();

		void InitBuffers();

		void InitRootSignature();

		void InitPipelineState();

		//Default texture
		void CreateDefaultTexture();

		Renderer::Scene* m_scene;

		ID3D12RootSignature* m_graphicsRootSignature;

		ID3D12PipelineState* m_graphicsPipelineState;

		ID3D12RootSignature* m_computeRootSignature;

		ID3D12PipelineState* m_computePipelineState;

		D3D12VertexBuffer* m_vertexBuffer;

		D3D12IndexBuffer* m_indexBuffer;

		D3D12UploadBuffer* m_uploadBuffer;

		D3D12UploadBuffer* m_VSUniform;

		D3D12UploadBuffer* m_PSUniform;

		D3D12StructBuffer* m_lightBuffer;

		HANDLE m_fenceEvent;

		std::array<uint64_t, 3> m_fenceValues;

		ID3D12Fence* m_fence;

		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain = nullptr;

		ID3D12Device* m_device = nullptr;
		
		D3D12GraphicsCmd* m_graphicsCmd = nullptr;

		D3D12GraphicsCmd* m_computeCmd = nullptr;

		uint32_t m_frameIndex;
		SceneUniformBuffer m_uniformBuffer;

		//CmdQueue for renderer,associate with swapchain
		//So thast cmdlist write to swapchain can be flushed
		//and properly synced.
		D3D12CmdQueue* m_renderCmdQueue;

		std::vector<PointLight> m_lightData;

		std::array<PointLight,1024> m_lights;
	};
}