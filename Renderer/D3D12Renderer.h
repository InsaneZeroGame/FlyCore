#pragma once
#include "stdafx.h"
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
#include "../Gameplay/System.h"
#include "../Gameplay/RenderComponent.h"
#include "D3D12UI.h"

namespace Renderer {

	struct PointLight
	{
		std::array<float,4> position;//view space
		std::array<float, 4> color;//view space
		float radius;
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

	struct PushConstants
	{
		glm::mat4 model;
		std::array<float,4> material;
	};
	


	class D3D12Renderer : public Interface::IRenderer, public Interface::IModule, public Gameplay::System<Gameplay::RenderComponent>
	{
	public:
		D3D12Renderer();

		~D3D12Renderer();

		// Inherited via IModule
		virtual void OnInit() override;

		virtual void OnUpdate() override;

		virtual void OnDestory() override;

		virtual void SetCamera(Gameplay::BaseCamera* p_camera) override;

		// Inherited via IRenderer
		void RenderScene() override;

		virtual void LoadUI(UI::UISystem* p_system) override;
	private:
		void InitSwapChain();

		void InitSyncPrimitive();

		void InitGraphicsContext();

		void SyncFrame();

		void InitBuffers();

		void InitRootSignature();

		void InitPipelineState();

		void InitRenderpass();

		void InitBuiltinMeshes();

		//Default texture
		void CreateDefaultTexture();



		typedef struct RenderPass
		{
			std::vector<D3D12_RENDER_PASS_RENDER_TARGET_DESC> mrt;
			D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depth;
		}RenderPass;

		RenderPass m_clusterForwardPass;

		ID3D12RootSignature* m_shadowPassRootSignature;

		ID3D12RootSignature* m_clusterForwardRootSignature;

		ID3D12RootSignature* m_finalOutputRootSignature;

		ID3D12PipelineState* m_graphicsPipelineState;

		ID3D12PipelineState* m_quadPipelineState;

		ID3D12RootSignature* m_lightCullRootSignature;

		ID3D12PipelineState* m_lightCullPipelineState;

		ID3D12RootSignature* m_ssrRootSignature;

		ID3D12PipelineState* m_ssrPipelineState;

		ID3D12PipelineState* m_shadowPassPipelineState;

		ID3D12PipelineState* m_skyboxPipelineState;

		ID3D12RootSignature* m_skyboxRootSignature;

		D3D12VertexBuffer* m_vertexBuffer;

		D3D12IndexBuffer* m_indexBuffer;

		D3D12UploadBuffer* m_uploadBuffer;

		D3D12UploadBuffer* m_VSUniform;

		D3D12UploadBuffer* m_PSUniform;

		D3D12StructBuffer* m_lightList;

		D3D12StructBuffer* m_lightBuffer;

		HANDLE m_fenceEvent;

		std::array<uint64_t, 3> m_fenceValues;

		ID3D12Fence* m_fence;

		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain = nullptr;

		ID3D12Device* m_device = nullptr;
		
		D3D12GraphicsCmd* m_graphicsCmd = nullptr;

		D3D12GraphicsCmd* m_lightCullCmd = nullptr;

		D3D12GraphicsCmd* m_ssrCmd = nullptr;
		
		uint32_t m_frameIndex;

		//CmdQueue for renderer,associate with swapchain
		//So thast cmdlist write to swapchain can be flushed
		//and properly synced.
		D3D12CmdQueue* m_renderCmdQueue;

		std::vector<PointLight> m_lightData;

		std::array<PointLight,1024> m_lights;

		struct BuiltinMesh
		{
			Gameplay::Mesh m_quadMesh;
			uint64_t m_quadVertexOffset;
			uint64_t m_quadIndexOffset;

		};

		BuiltinMesh m_frameQuad;

		BuiltinMesh m_skyBoxMesh;

		uint32_t m_width;

		uint32_t m_height;

		D3D12Texture2D* m_defaultTexture;

		//D3D12Texture2D* m_ssr;

		D3D12TextureCube* m_skyBox;

		bool m_isFirstFrame;

		uint64_t m_vertexOffsetInByte;
		uint64_t m_indexOffsetInByte;

		D3D12AnimBuffer* m_animBuffer;

		UI::D3D12UISystem* m_ui;

		

};
};
