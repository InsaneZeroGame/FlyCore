#include "stdafx.h"
#include "D3D12Renderer.h"
#include <D3Dcompiler.h>
#include "../Utility/AssetLoader.h"
#include "D3D12DescManager.h"
#include "../Gameplay/Entity.h"
#include "../Gameplay/Director.h"


#define CAMERA_UNIFORM_ROOT_INDEX 0
#define LIGHT_UAV_ROOT_INDEX 1
#define LIGHT_BUFFER_ROOT_INDEX 2
#define FINAL_OUTPUT_TEX_ROOT_INDEX 1
#define DIFFUSE_MAP 3
#define SHADOW_MAP 4
#define PUSH_CONSTANTS 5
#define COLOR_PASS_ANIM_ROOT_INDEX 6
#define SHADOW_PASS_ANIM_ROOT_INDEX 2
Renderer::D3D12Renderer::D3D12Renderer():
    m_device(D3D12Device::GetDevice()),
    m_swapChain(nullptr),
    m_frameIndex(0),
	m_graphicsCmd(new D3D12GraphicsCmd(Constants::SWAPCHAIN_BUFFER_COUNT)),
	m_lightCullCmd(new D3D12GraphicsCmd(Constants::COMPUTE_CMD_COUNT)),
	m_ssrCmd(new D3D12GraphicsCmd(Constants::COMPUTE_CMD_COUNT)),
	m_VSUniform(new D3D12UploadBuffer(Utility::AlignTo256(sizeof(SceneUniformData)))),
	m_renderCmdQueue(new D3D12CmdQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)),
    m_lightList(nullptr),
	m_width(0),
	m_height(0),
	m_quadPipelineState(nullptr),
	m_lightCullPipelineState(nullptr),
	m_graphicsPipelineState(nullptr),
	m_vertexBuffer(new D3D12VertexBuffer(Constants::VERTEX_BUFFER_SIZE)),
	m_indexBuffer(new D3D12IndexBuffer(Constants::VERTEX_BUFFER_SIZE)),
	m_uploadBuffer(new D3D12UploadBuffer(Constants::MAX_CONST_BUFFER_VIEW_SIZE)),
	m_clusterForwardRootSignature(nullptr),
	m_shadowPassRootSignature(nullptr),
	m_isFirstFrame(true),
	m_vertexOffsetInByte(0),
	m_indexOffsetInByte(0),
	m_animBuffer(nullptr),
	m_ui(nullptr)
{
}

Renderer::D3D12Renderer::~D3D12Renderer()
{
}

void Renderer::D3D12Renderer::OnInit()
{
	ASSERT(m_window, "A window must be set before create swapchain\n");
	InitSwapChain();
    InitGraphicsContext();
    InitSyncPrimitive();
	InitRenderpass();
    InitBuffers();
    InitRootSignature();
    InitPipelineState();
}


static std::array<float,4> debugColor[10] = 
{
	{1.0f,0.0f,0.0f,1.0f},
	{0.0f,1.0f,0.0f,1.0f},
	{0.0f,0.0f,1.0f,1.0f},
	{1.0f,0.0f,0.0f,1.0f},
	{0.0f,1.0f,0.0f,1.0f},
	{0.0f,0.0f,1.0f,1.0f},
	{1.0f,0.0f,0.0f,1.0f},
	{0.0f,1.0f,0.0f,1.0f},
	{0.0f,0.0f,1.0f,1.0f},
	{0.75f,0.75f,0.75f,1.0f},


};

void Renderer::D3D12Renderer::OnUpdate()
{
	auto& l_director = Gameplay::Director::GetDirector();
	if (m_mainCamera->IsCameraUpdated() || m_isFirstFrame)
	{
		m_VSUniform->ResetBuffer();
		m_mainCamera->UpdateCamera();

		auto shadowMatrix = glm::lookAtLH(glm::vec3(10.01, 15.0f, 2.0), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));



		SceneUniformData l_data = {
			m_mainCamera->GetProj(),
			m_mainCamera->GetView(),
			m_mainCamera->GetInverseProj(),
			m_mainCamera->GetShadowProj() * shadowMatrix,
			{m_mainCamera->GetNear(),m_mainCamera->GetFar(),0.0,0.0}
		};

		m_VSUniform->CopyData(&l_data, Utility::AlignTo256(sizeof(SceneUniformData)));
		m_isFirstFrame = false;
	}
	//Light cull 
	{
		using namespace Constants;
		m_lightCullCmd->Reset(0, m_lightCullPipelineState);
		ID3D12GraphicsCommandList* l_computeCmdList = *m_lightCullCmd;
		l_computeCmdList->SetPipelineState(m_lightCullPipelineState);
		l_computeCmdList->SetComputeRootSignature(m_lightCullRootSignature);
		l_computeCmdList->SetComputeRootConstantBufferView(CAMERA_UNIFORM_ROOT_INDEX, m_VSUniform->GetGpuVirtualAddress());
        l_computeCmdList->SetComputeRootUnorderedAccessView(LIGHT_UAV_ROOT_INDEX, m_lightList->GetGpuVirtualAddress());
		l_computeCmdList->SetComputeRootShaderResourceView(LIGHT_BUFFER_ROOT_INDEX, m_lightBuffer->GetGpuVirtualAddress());
		l_computeCmdList->Dispatch(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y, WORK_GROUP_SIZE_Z);
		m_lightCullCmd->Close();
		m_lightCullCmd->Flush(true);
	}
	

	auto& l_graphicsContext = D3D12GraphicsContext::GetContext();
	ID3D12GraphicsCommandList4* l_graphicsCmdList = *m_graphicsCmd;
	m_graphicsCmd->Reset(m_frameIndex, m_graphicsPipelineState);
	D3D12DepthBuffer* l_shadowMap = l_graphicsContext.GetDepthBuffer("ShadowMap");
	D3D12DepthBuffer* l_depthBuffer = l_graphicsContext.GetDepthBuffer("DepthBuffer");
	const auto& all_entities = Gameplay::EntityManager::GetManager().GetAllEntities();

	//Shadow pass
	{
		l_graphicsCmdList->SetPipelineState(m_shadowPassPipelineState);
		l_graphicsCmdList->OMSetRenderTargets(0, nullptr, false, &l_shadowMap->GetDSV()->cpuHandle);
		l_graphicsCmdList->RSSetViewports(1, &ShadowPassViewPort);
		l_graphicsCmdList->RSSetScissorRects(1, &ShadowPassRect);
		l_graphicsCmdList->ClearDepthStencilView(l_shadowMap->GetDSV()->cpuHandle,D3D12_CLEAR_FLAG_DEPTH,1.0f,0,1, &ShadowPassRect);
		l_graphicsCmdList->IASetIndexBuffer(&m_indexBuffer->GetIndexBufferView());
		l_graphicsCmdList->SetGraphicsRootSignature(m_shadowPassRootSignature);
		l_graphicsCmdList->IASetVertexBuffers(0, 1, &m_vertexBuffer->GetVertexBufferView());
		l_graphicsCmdList->SetGraphicsRootConstantBufferView(CAMERA_UNIFORM_ROOT_INDEX, m_VSUniform->GetGpuVirtualAddress());
		l_graphicsCmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		uint64_t l_animIndex = 0;
		for (Gameplay::Entity i = 0; i < all_entities.size(); ++i)
		{
			if (!all_entities[i]) continue;
			if (m_systemEntites.find(i) == m_systemEntites.end()) continue;
			auto& l_entity_meshes = m_systemEntites[i]->GetComponent()->GetMeshes();

			l_graphicsCmdList->SetGraphicsRoot32BitConstants(1, 16, &l_director.Query(i)->GetComponent()->GetModelMatrix(), 0);
			for (int j = 0; j < l_entity_meshes.size(); ++j)
			{
				Gameplay::Mesh* l_mesh = l_entity_meshes[j];
				if (l_mesh->m_anim)
				{
					l_graphicsCmdList->SetGraphicsRootConstantBufferView(SHADOW_PASS_ANIM_ROOT_INDEX, m_animBuffer->GetActorAnimBufferLocation(l_animIndex));
					l_animIndex++;
				}

				l_graphicsCmdList->DrawIndexedInstanced(
					static_cast<uint32_t>(l_mesh->m_indices.size()),
					1, 
					static_cast<uint32_t>(l_mesh->m_indexOffset + m_systemEntites[i]->GetComponent()->m_componentIndexOffset),
					static_cast<uint32_t>(l_mesh->m_vertexOffset + m_systemEntites[i]->GetComponent()->m_componentVertexOffset), 0);
			}
		}
		
		
		
		DepthToShaderResource.Transition.pResource = l_shadowMap->GetResource();
		l_graphicsCmdList->ResourceBarrier(1, &DepthToShaderResource);
	}

	l_graphicsContext.BeginRender(m_frameIndex);

	//Skybox pass
	{
		l_graphicsCmdList->SetPipelineState(m_skyboxPipelineState);
		l_graphicsCmdList->SetGraphicsRootSignature(m_skyboxRootSignature);
		l_graphicsCmdList->OMSetRenderTargets(1, &l_graphicsContext.GetRenderTarget("Light")->GetRTV()->cpuHandle, false, nullptr);
		l_graphicsCmdList->IASetIndexBuffer(&m_indexBuffer->GetIndexBufferView());
		l_graphicsCmdList->IASetVertexBuffers(1, 1, &m_vertexBuffer->GetVertexBufferView());
		l_graphicsCmdList->SetGraphicsRootConstantBufferView(CAMERA_UNIFORM_ROOT_INDEX, m_VSUniform->GetGpuVirtualAddress());
		ID3D12DescriptorHeap* l_srvHeap[] = { D3D12DescManager::GetDescManager().GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->GetHeap() };
		l_graphicsCmdList->SetDescriptorHeaps(1, l_srvHeap);
		l_graphicsCmdList->SetGraphicsRootDescriptorTable(1, m_skyBox->GetSRV()->gpuHandle);
		l_graphicsCmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		l_graphicsCmdList->DrawIndexedInstanced(static_cast<uint32_t>(m_skyBoxMesh.m_quadMesh.m_indices.size()), 1, static_cast<uint32_t>(m_skyBoxMesh.m_quadIndexOffset), static_cast<uint32_t>(m_skyBoxMesh.m_quadVertexOffset), 0);
	}


	//Forward Pass
	{
		//Resource Trasition
		l_graphicsContext.BeginRenderpass(m_clusterForwardPass.mrt, &m_clusterForwardPass.depth);
		l_graphicsCmdList->SetPipelineState(m_graphicsPipelineState);
		l_graphicsCmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// Set necessary state.
		l_graphicsCmdList->SetGraphicsRootSignature(m_clusterForwardRootSignature);
		l_graphicsCmdList->IASetIndexBuffer(&m_indexBuffer->GetIndexBufferView());
		l_graphicsCmdList->IASetVertexBuffers(0, 1, &m_vertexBuffer->GetVertexBufferView());
		l_graphicsCmdList->SetGraphicsRootConstantBufferView(CAMERA_UNIFORM_ROOT_INDEX, m_VSUniform->GetGpuVirtualAddress());
		l_graphicsCmdList->SetGraphicsRootShaderResourceView(LIGHT_UAV_ROOT_INDEX, m_lightList->GetGpuVirtualAddress());
		l_graphicsCmdList->SetGraphicsRootShaderResourceView(LIGHT_BUFFER_ROOT_INDEX, m_lightBuffer->GetGpuVirtualAddress());
		ID3D12DescriptorHeap* l_srvHeap[] = { D3D12DescManager::GetDescManager().GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->GetHeap() };
		l_graphicsCmdList->SetDescriptorHeaps(1, l_srvHeap);
		l_graphicsCmdList->SetGraphicsRootDescriptorTable(DIFFUSE_MAP, m_defaultTexture->GetSRV()->gpuHandle);
		l_graphicsCmdList->SetGraphicsRootDescriptorTable(SHADOW_MAP, l_shadowMap->GetSRV()->gpuHandle);
		uint32_t uniformDebugColor = 0;
		uint64_t l_animIndex = 0;
		static int animPlayRate = 0;
		animPlayRate++;
		for (auto i = 0; i < all_entities.size(); ++i)
		{
			if (!all_entities[i]) continue;
			if (m_systemEntites.find(i) == m_systemEntites.end()) continue;
			auto& l_entity_meshes = m_systemEntites[i]->GetComponent()->GetMeshes();

			l_graphicsCmdList->SetGraphicsRoot32BitConstants(PUSH_CONSTANTS, 16, &l_director.Query(i)->GetComponent()->GetModelMatrix(), 0);
			
			for (int j = 0; j < l_entity_meshes.size(); ++j)
			{
				Gameplay::Mesh* l_mesh = l_entity_meshes[j];
				if (l_mesh->m_anim)
				{
					if (animPlayRate % 1 == 0)
					{
						auto animFrame = (l_mesh->m_anim->m_currentFrameIndex++) % l_mesh->m_anim->m_totalFrameCount;
						m_animBuffer->UpdateActorAnim(l_animIndex, l_mesh->m_anim->bones[animFrame].data());
					}
					
					l_graphicsCmdList->SetGraphicsRootConstantBufferView(COLOR_PASS_ANIM_ROOT_INDEX, m_animBuffer->GetActorAnimBufferLocation(l_animIndex));
					l_animIndex++;
				}
				//Update Roughness
				l_graphicsCmdList->SetGraphicsRoot32BitConstants(PUSH_CONSTANTS, 3, debugColor[9].data(), 16);
				l_graphicsCmdList->SetGraphicsRoot32BitConstants(PUSH_CONSTANTS, 1, &l_mesh->GetRoughness(), 19);

				l_graphicsCmdList->DrawIndexedInstanced(
					static_cast<uint32_t>(l_mesh->m_indices.size()),
					1,
					static_cast<uint32_t>(l_mesh->m_indexOffset + m_systemEntites[i]->GetComponent()->m_componentIndexOffset),
					static_cast<uint32_t>(l_mesh->m_vertexOffset + m_systemEntites[i]->GetComponent()->m_componentVertexOffset), 0);
			}
		}
		l_graphicsCmdList->EndRenderPass();
		//Resource Trasition
		ShaderResourceToDepth.Transition.pResource = l_shadowMap->GetResource();
		l_graphicsCmdList->ResourceBarrier(1, &ShaderResourceToDepth);
	}




	{
		m_ssrCmd->Reset(0);
		ID3D12GraphicsCommandList* l_ssrCmd = *m_ssrCmd;
		l_ssrCmd->SetPipelineState(m_ssrPipelineState);
		l_ssrCmd->SetComputeRootSignature(m_ssrRootSignature);
		ID3D12DescriptorHeap* l_srvHeap[] = { D3D12DescManager::GetDescManager().GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->GetHeap() };
		l_ssrCmd->SetDescriptorHeaps(1, l_srvHeap);
		l_ssrCmd->SetComputeRootDescriptorTable(0, m_ssr->GetSRV()->gpuHandle);
		l_ssrCmd->Dispatch(60, 135, 1);
		m_ssrCmd->Close();
		m_ssrCmd->Flush(true);
	}

	//Frame Quad
	{
		l_graphicsContext.TransitRenderTargets({ "Light","Normal","Specular" }, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		//DepthToShaderResource.Transition.pResource = l_depthBuffer->GetResource();
		//l_graphicsCmdList->ResourceBarrier(1, &DepthToShaderResource);
		//Deferred Pass
		l_graphicsContext.BeginSwapchainOutputPass(m_frameIndex);
		l_graphicsCmdList->SetPipelineState(m_quadPipelineState);
		l_graphicsCmdList->SetGraphicsRootSignature(m_clusterForwardRootSignature);
		l_graphicsCmdList->IASetIndexBuffer(&m_indexBuffer->GetIndexBufferView());
		l_graphicsCmdList->IASetVertexBuffers(0, 1, &m_vertexBuffer->GetVertexBufferView());
		l_graphicsCmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ID3D12DescriptorHeap* l_srvHeap[] = { D3D12DescManager::GetDescManager().GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->GetHeap() };
		l_graphicsCmdList->SetGraphicsRootSignature(m_finalOutputRootSignature);
		l_graphicsCmdList->SetDescriptorHeaps(1, l_srvHeap);
		l_graphicsCmdList->SetGraphicsRootDescriptorTable(FINAL_OUTPUT_TEX_ROOT_INDEX, l_graphicsContext.GetRenderTarget("Light")->GetSRV()->gpuHandle);
		l_graphicsCmdList->DrawIndexedInstanced(static_cast<uint32_t>(m_frameQuad.m_quadMesh.m_indices.size()), 1, static_cast<uint32_t>(m_frameQuad.m_quadIndexOffset), static_cast<uint32_t>(m_frameQuad.m_quadVertexOffset), 0);
		l_graphicsContext.TransitRenderTargets({ "Light","Normal","Specular" }, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		//ShaderResourceToDepth.Transition.pResource = l_depthBuffer->GetResource();
		//l_graphicsCmdList->ResourceBarrier(1, &ShaderResourceToDepth);
		m_ui->RenderUI(l_graphicsCmdList);

		l_graphicsCmdList->EndRenderPass();
	}

	l_graphicsContext.EndRender();
	m_graphicsCmd->Close();
    // Execute the command list.
	ID3D12CommandList* l_lists[] = {l_graphicsCmdList};
	m_renderCmdQueue->GetQueue()->ExecuteCommandLists(1, l_lists);
    m_swapChain->Present(1, 0);
    SyncFrame();
}



void Renderer::D3D12Renderer::RenderScene()
{
}

void Renderer::D3D12Renderer::InitSwapChain()
{
    // Obtain the DXGI factory
    using namespace Microsoft::WRL;
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    ASSERT_SUCCEEDED(CreateDXGIFactory2(0, MY_IID_PPV_ARGS(&dxgiFactory)));

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_window->GetWidth();
    swapChainDesc.Height = m_window->GetHeight();
    swapChainDesc.Format = Constants::SwapChainFormat;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = Constants::SWAPCHAIN_BUFFER_COUNT;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) // Win32
    dxgiFactory->CreateSwapChainForHwnd(m_renderCmdQueue->GetQueue(), m_window->GetWin32Window(), &swapChainDesc, nullptr, nullptr, &swapChain1);
    swapChain1.As(&m_swapChain);
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
#else // UWP
    ASSERT_SUCCEEDED(dxgiFactory->CreateSwapChainForCoreWindow(g_CommandManager.GetCommandQueue(), (IUnknown*)GameCore::g_window.Get(), &swapChainDesc, nullptr, &s_SwapChain1));
#endif

#if CONDITIONALLY_ENABLE_HDR_OUTPUT && defined(NTDDI_WIN10_RS2) && (NTDDI_VERSION >= NTDDI_WIN10_RS2)
    {
        IDXGISwapChain4* swapChain = (IDXGISwapChain4*)s_SwapChain1;
        ComPtr<IDXGIOutput> output;
        ComPtr<IDXGIOutput6> output6;
        DXGI_OUTPUT_DESC1 outputDesc;
        UINT colorSpaceSupport;

        // Query support for ST.2084 on the display and set the color space accordingly
        if (SUCCEEDED(swapChain->GetContainingOutput(&output)) &&
            SUCCEEDED(output.As(&output6)) &&
            SUCCEEDED(output6->GetDesc1(&outputDesc)) &&
            outputDesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 &&
            SUCCEEDED(swapChain->CheckColorSpaceSupport(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020, &colorSpaceSupport)) &&
            (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT) &&
            SUCCEEDED(swapChain->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)))
        {
            g_bEnableHDROutput = true;
        }
    }
#endif
}

void Renderer::D3D12Renderer::InitSyncPrimitive()
{
    m_fenceValues[0] = 1;
    m_fenceValues[1] = 0;
    m_fenceValues[2] = 0;
    m_fenceEvent = CreateEvent(0, false, false, 0);
    ASSERT_SUCCEEDED(m_device->CreateFence(m_fenceValues[0], D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, MY_IID_PPV_ARGS(&m_fence)));
}

void Renderer::D3D12Renderer::InitGraphicsContext()
{
	D3D12GraphicsContext& l_context = D3D12GraphicsContext::GetContext();
	m_width = m_window->GetWidth();
	m_height = m_window->GetHeight();
	l_context.InitSwapchainOutputTarget(m_width, m_height, m_swapChain.Get());
	l_context.SetGraphicsCmdList(*m_graphicsCmd);
	l_context.AddRenderTargets("Light", m_width, m_height, nullptr, DXGI_FORMAT::DXGI_FORMAT_R10G10B10A2_UNORM);
	l_context.AddRenderTargets("Normal", m_width, m_height, nullptr, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT);
	l_context.AddRenderTargets("Specular", m_width, m_height, nullptr, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT);
	l_context.AddDepthBuffer("DepthBuffer", m_width, m_height);
	l_context.AddDepthBuffer("ShadowMap", ShadowMapWidth, ShadowMapHeight);

}

void Renderer::D3D12Renderer::SyncFrame()
{
    //1.Get Current Available Backbuffer
    m_renderCmdQueue->GetQueue()->Signal(m_fence, m_fenceValues[m_frameIndex]);

    auto currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    //2.Query if cmdlist to this buffer is completed.
    if (m_fence->GetCompletedValue() < m_fenceValues[currentBackBufferIndex])
    {
        m_fence->SetEventOnCompletion(m_fenceValues[currentBackBufferIndex], m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
    m_fenceValues[currentBackBufferIndex] = m_fenceValues[m_frameIndex] + 1;
    m_frameIndex = currentBackBufferIndex;
}

void Renderer::D3D12Renderer::SetCamera(Gameplay::BaseCamera* p_camera)
{
	m_uploadBuffer->ResetBuffer();

	m_mainCamera = p_camera;
	std::array<PointLight, 256> l_lights;
	m_lightBuffer = new D3D12StructBuffer(l_lights.size(), sizeof(PointLight));

	for (auto i = 0; i < l_lights.size(); ++i)
	{
		l_lights[i] = PointLight();
		l_lights[i].isActive = false;
	}

	struct Color
	{
		float data[4];
	};


	Color l_colors[] =
	{
		{1.0f,0.0f,0.0f,1.0f},
		{0.0f,1.0f,0.0f,1.0f},
		{0.0f,0.0f,1.0f,1.0f},
		{1.0f,0.0f,1.0f,1.0f},
		{0.0f,1.0f,1.0f,1.0f},
		{1.0f,0.0f,1.0f,1.0f},

	};
	float step = 10.0f / 25;

	for (auto i = 0; i < 16; ++i)
	{
		for (auto j = 0; j < 16; ++j)
		{
			auto lightPosView = glm::vec4(25 * Utility::RandomFloat_11(), 6.0 * Utility::RandomFloat_01(), 25 * Utility::RandomFloat_11(), 1.0);
			//auto lightPosView = m_uniformBuffer.m_view * glm::vec4(0, 0.5, 0, 1.0f);
			l_lights[i * 16 + j].isActive = true;
			l_lights[i * 16 + j].position[0] = lightPosView.x;
			l_lights[i * 16 + j].position[1] = lightPosView.y;
			l_lights[i * 16 + j].position[2] = lightPosView.z;
			l_lights[i * 16 + j].position[3] = lightPosView.w;
			l_lights[i * 16 + j].color[0] = l_colors[rand() % 6].data[0];
			l_lights[i * 16 + j].color[1] = l_colors[rand() % 6].data[1];
			l_lights[i * 16 + j].color[2] = l_colors[rand() % 6].data[2];
			l_lights[i * 16 + j].color[3] = l_colors[rand() % 6].data[3];
			l_lights[i * 16 + j].radius = 8.0;
			l_lights[i * 16 + j].attenutation = 3.0f;
		}
	}

	m_uploadBuffer->CopyData(l_lights.data(), l_lights.size() * sizeof(PointLight));
	auto& l_graphicsContext = D3D12GraphicsCmdContext::GetContext();
	l_graphicsContext.Begin(nullptr);
	l_graphicsContext.TransitResourceState(m_lightBuffer->GetResource(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COPY_DEST);
	l_graphicsContext.CopyBufferData(m_lightBuffer->GetResource(), 0, m_uploadBuffer->GetResource(), 0, sizeof(PointLight) * l_lights.size());
	l_graphicsContext.TransitResourceState(m_lightBuffer->GetResource(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ);
	l_graphicsContext.End(true);
	m_uploadBuffer->ResetBuffer();
}



void Renderer::D3D12Renderer::InitBuffers()
{
	InitBuiltinMeshes();
	using namespace Gameplay;

	const auto& all_entities = Gameplay::EntityManager::GetManager().GetAllEntities();
	auto l_vertexOffset = m_vertexOffsetInByte;
	auto l_indexOffset = m_indexOffsetInByte;

 	for (Gameplay::Entity i = 0; i < all_entities.size(); ++i)
	{
		if (!all_entities[i]) continue;
		if (m_systemEntites.find(i) == m_systemEntites.end()) continue;

		auto& l_entity_meshes = m_systemEntites[i]->GetComponent()->GetMeshes();
		m_systemEntites[i]->GetComponent()->m_componentVertexOffset = m_vertexOffsetInByte/sizeof(Vertex);
		for (auto j = 0; j < l_entity_meshes.size(); ++j)
		{
			auto l_vertexSize = (UINT)sizeof(l_entity_meshes[j]->m_vertices[0]) * static_cast<uint32_t>(l_entity_meshes[j]->m_vertices.size());
			m_uploadBuffer->CopyData((void*)l_entity_meshes[j]->m_vertices.data(), l_vertexSize);
			m_vertexOffsetInByte += l_vertexSize;
		}
	}

	for (Gameplay::Entity i = 0; i < all_entities.size(); ++i)
	{
		if (!all_entities[i]) continue;
		if (m_systemEntites.find(i) == m_systemEntites.end()) continue;

		auto& l_entity_meshes = m_systemEntites[i]->GetComponent()->GetMeshes();
		m_systemEntites[i]->GetComponent()->m_componentIndexOffset = m_indexOffsetInByte/sizeof(uint32_t);

		for (auto j = 0; j < l_entity_meshes.size(); j++)
		{
			auto l_indexSize = (UINT)sizeof(l_entity_meshes[j]->m_indices[0]) * static_cast<uint32_t>(l_entity_meshes[j]->m_indices.size());
			m_uploadBuffer->CopyData((void*)l_entity_meshes[j]->m_indices.data(), l_indexSize);
			m_indexOffsetInByte += l_indexSize;
		}
	}

	uint64_t animCount = 0;

	for (Gameplay::Entity i = 0; i < all_entities.size(); ++i)
	{
		if (!all_entities[i]) continue;
		if (m_systemEntites.find(i) == m_systemEntites.end()) continue;

		auto& l_entity_meshes = m_systemEntites[i]->GetComponent()->GetMeshes();
		for (int j = 0; j < l_entity_meshes.size(); ++j)
		{
			Gameplay::Mesh* l_mesh = l_entity_meshes[j];
			if (l_mesh->m_anim)
			{
				animCount++;
			}
		}
	}
	
	m_animBuffer = new D3D12AnimBuffer(animCount);
	
	auto& l_graphicsContext = D3D12GraphicsCmdContext::GetContext();
	l_graphicsContext.Begin(nullptr);
    l_graphicsContext.UploadVertexBuffer(m_vertexBuffer, l_vertexOffset, m_uploadBuffer, 0, m_vertexOffsetInByte-l_vertexOffset);
    l_graphicsContext.UploadVertexBuffer(m_indexBuffer, l_indexOffset, m_uploadBuffer, m_vertexOffsetInByte - l_vertexOffset, m_indexOffsetInByte - l_indexOffset);
    l_graphicsContext.TransitResourceState(m_vertexBuffer->GetResource(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    l_graphicsContext.TransitResourceState(m_indexBuffer->GetResource(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_INDEX_BUFFER);
    l_graphicsContext.End(true);

	CreateDefaultTexture();
    m_lightList = new D3D12StructBuffer(1024, sizeof(LightList));
}

void Renderer::D3D12Renderer::InitRootSignature()
{
	using namespace Microsoft::WRL;

	//STATIC SAMPLER DESC
	D3D12_STATIC_SAMPLER_DESC l_defaultSampler = {};
	l_defaultSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	l_defaultSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	l_defaultSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	l_defaultSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	l_defaultSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	
	//Skybox

	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;

		D3D12_ROOT_PARAMETER l_cameraUniform = {};
		l_cameraUniform.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		l_cameraUniform.Descriptor = { 0,0 };
		l_cameraUniform.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		

		D3D12_ROOT_PARAMETER l_skyboxTexture = {};
		l_skyboxTexture.DescriptorTable.NumDescriptorRanges = 1;
		l_skyboxTexture.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		D3D12_DESCRIPTOR_RANGE l_range = {};
		l_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		l_range.NumDescriptors = 1;
		l_range.BaseShaderRegister = 0;
		l_range.RegisterSpace = 0;
		l_range.OffsetInDescriptorsFromTableStart = 0;
		l_skyboxTexture.DescriptorTable.pDescriptorRanges = &l_range;
		l_skyboxTexture.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		std::vector<D3D12_ROOT_PARAMETER> l_rootParameters =
		{
			l_cameraUniform,l_skyboxTexture
		};

		D3D12_STATIC_SAMPLER_DESC l_CubeSampler = {};
		l_CubeSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		l_CubeSampler.Filter = D3D12_FILTER_ANISOTROPIC;
		l_CubeSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		l_CubeSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		l_CubeSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		l_CubeSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

		rootSignatureDesc.Init(static_cast<uint32_t>(l_rootParameters.size()), l_rootParameters.data(), 1, &l_CubeSampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_skyboxRootSignature));

	}


    // Create Graphics RS
	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;



		D3D12_ROOT_PARAMETER l_cameraUniform = {};
		l_cameraUniform.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		l_cameraUniform.Descriptor = { 0,0 };
		l_cameraUniform.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_PARAMETER l_lightUAV = {};
		l_lightUAV.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		l_lightUAV.Descriptor = { 1,0 };
		l_lightUAV.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_ROOT_PARAMETER l_lightBuffer = {};
		l_lightBuffer.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		l_lightBuffer.Descriptor = { 2,0 };
		l_lightBuffer.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_ROOT_PARAMETER l_diffuseMap = {};
		l_diffuseMap.DescriptorTable.NumDescriptorRanges = 1;
		l_diffuseMap.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		D3D12_DESCRIPTOR_RANGE l_range = {};
		l_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		l_range.NumDescriptors = 1;
		l_range.BaseShaderRegister = 3;
		l_range.RegisterSpace = 0;
		l_range.OffsetInDescriptorsFromTableStart = 0;
		l_diffuseMap.DescriptorTable.pDescriptorRanges = &l_range;
		l_diffuseMap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


		D3D12_ROOT_PARAMETER l_shadowMap = {};
		l_shadowMap.DescriptorTable.NumDescriptorRanges = 1;
		l_shadowMap.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		D3D12_DESCRIPTOR_RANGE l_shadow_map_range = {};
		l_shadow_map_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		l_shadow_map_range.NumDescriptors = 1;
		l_shadow_map_range.BaseShaderRegister = 4;
		l_shadow_map_range.RegisterSpace = 0;
		l_shadow_map_range.OffsetInDescriptorsFromTableStart = 0;
		l_shadowMap.DescriptorTable.pDescriptorRanges = &l_shadow_map_range;
		l_shadowMap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_ROOT_PARAMETER l_pushConstants = {};

		l_pushConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		l_pushConstants.Constants = {5,0,16 + 4};
		l_pushConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_PARAMETER l_animBuffer = {};
		l_animBuffer.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		l_animBuffer.Descriptor.RegisterSpace = 0;
		l_animBuffer.Descriptor.ShaderRegister = 6;
		l_animBuffer.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


		std::vector<D3D12_ROOT_PARAMETER> l_rootParameters =
		{
			l_cameraUniform,
			l_lightUAV,
			l_lightBuffer,
			l_diffuseMap,
			l_shadowMap,
			l_pushConstants,
			l_animBuffer
		};
		

		D3D12_STATIC_SAMPLER_DESC l_shadowSampler = {};
		l_shadowSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		l_shadowSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		l_shadowSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		l_shadowSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		l_shadowSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		l_shadowSampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		l_shadowSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		l_shadowSampler.ShaderRegister = 1;

		std::vector<D3D12_STATIC_SAMPLER_DESC> samplers = { l_defaultSampler ,l_shadowSampler };



		rootSignatureDesc.Init(static_cast<uint32_t>(l_rootParameters.size()), l_rootParameters.data(), static_cast<uint32_t>(samplers.size()), samplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_clusterForwardRootSignature));

	}

	//Shadow Map Pass
	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;

		D3D12_ROOT_PARAMETER l_cameraUniform = {};
		l_cameraUniform.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		l_cameraUniform.Descriptor = { 0,0 };
		l_cameraUniform.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_PARAMETER l_pushConstants = {};

		l_pushConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		l_pushConstants.Constants = { 5,0,16 + 4 };
		l_pushConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_PARAMETER l_animBuffer = {};
		l_animBuffer.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		l_animBuffer.Descriptor.RegisterSpace = 0;
		l_animBuffer.Descriptor.ShaderRegister = 6;
		l_animBuffer.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		std::vector<D3D12_ROOT_PARAMETER> l_rootParameters =
		{
			l_cameraUniform,
			l_pushConstants,
			l_animBuffer
		};

		rootSignatureDesc.Init(static_cast<uint32_t>(l_rootParameters.size()), l_rootParameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_shadowPassRootSignature));

	}

	//Final Output
	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;

		D3D12_ROOT_PARAMETER l_cameraUniform = {};
		l_cameraUniform.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		l_cameraUniform.Descriptor = { 0,0 };
		l_cameraUniform.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_PARAMETER l_finalOutputTextures = {};
		l_finalOutputTextures.DescriptorTable.NumDescriptorRanges = 1;
		l_finalOutputTextures.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		D3D12_DESCRIPTOR_RANGE l_range = {};
		l_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		//+1 is for the depth buffer that used in screen space related tech.
		l_range.NumDescriptors = static_cast<uint32_t>(m_clusterForwardPass.mrt.size() + 1);
		l_range.BaseShaderRegister = 0;
		l_range.RegisterSpace = 0;
		l_range.OffsetInDescriptorsFromTableStart = 0;
		l_finalOutputTextures.DescriptorTable.pDescriptorRanges = &l_range;
		l_finalOutputTextures.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		std::vector<D3D12_ROOT_PARAMETER> l_rootParameters =
		{
			l_cameraUniform,
			l_finalOutputTextures
		};

		

		rootSignatureDesc.Init(static_cast<uint32_t>(l_rootParameters.size()), l_rootParameters.data(), 1, &l_defaultSampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_finalOutputRootSignature));

	}
   
	// Create Light RS
	{
		CD3DX12_ROOT_SIGNATURE_DESC l_computeRootSignatureDesc;

		D3D12_ROOT_PARAMETER l_cameraUniform = {};
		l_cameraUniform.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		l_cameraUniform.Descriptor = { 0,0 };
		l_cameraUniform.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_PARAMETER l_lightUAV = {};
        l_lightUAV.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
        l_lightUAV.Descriptor = { 0,0 };
        l_lightUAV.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_PARAMETER l_lightBuffer = {};
		l_lightBuffer.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		l_lightBuffer.Descriptor = { 1,0 };
		l_lightBuffer.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		std::vector<D3D12_ROOT_PARAMETER> l_rootParameters =
		{
			l_cameraUniform,
            l_lightUAV,
			l_lightBuffer
		};

		l_computeRootSignatureDesc.Init(static_cast<uint32_t>(l_rootParameters.size()), l_rootParameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3D12SerializeRootSignature(&l_computeRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_lightCullRootSignature));
	}

	//Create SSR RS
	{
		CD3DX12_ROOT_SIGNATURE_DESC l_ssrRootSignatureDesc = {};

		D3D12_DESCRIPTOR_RANGE l_range = {};
		l_range.BaseShaderRegister = 0;
		l_range.NumDescriptors = 1;
		l_range.OffsetInDescriptorsFromTableStart = 0;
		l_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		l_range.RegisterSpace = 0;

		D3D12_ROOT_PARAMETER l_ssrOuput = {};
		l_ssrOuput.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		l_ssrOuput.DescriptorTable.NumDescriptorRanges = 1;
		l_ssrOuput.DescriptorTable.pDescriptorRanges = &l_range;

		std::vector<D3D12_ROOT_PARAMETER> l_rootParameters =
		{
			l_ssrOuput
		};

		l_ssrRootSignatureDesc.Init(l_rootParameters.size(), l_rootParameters.data(), 0, nullptr);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3D12SerializeRootSignature(&l_ssrRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_ssrRootSignature));
	}
}

void Renderer::D3D12Renderer::CreateDefaultTexture()
{
	auto& l_graphicsContext = D3D12GraphicsCmdContext::GetContext();
	Gameplay::Texture l_texture;
	Utility::AssetLoader::LoadTextureFromFile("C:\\Dev\\FlyCore\\Assets\\uv_texture.png", l_texture);
	m_uploadBuffer->ResetBuffer();
	m_defaultTexture = new D3D12Texture2D(l_texture.width, l_texture.height);
	m_ssr = new D3D12Texture2D(l_texture.width, l_texture.height);

	m_defaultTexture->SetName(L"Default Texture");
	m_uploadBuffer->CopyData(l_texture.data, l_texture.size);
	l_graphicsContext.Begin(nullptr);
	l_graphicsContext.CopyTextureData(
		m_defaultTexture->GetResource(),
		m_uploadBuffer->GetResource(),
		l_texture.width, l_texture.height, DXGI_FORMAT_R8G8B8A8_UNORM);
	l_graphicsContext.TransitResourceState(m_defaultTexture->GetResource(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	l_graphicsContext.End(true);

	m_uploadBuffer->ResetBuffer();
	std::array<Gameplay::Texture, 6> l_skybox_textures;
	Utility::AssetLoader::LoadTextureFromFile("C:\\Dev\\FlyCore\\Assets\\skybox\\left.jpg", l_skybox_textures[0]);
	Utility::AssetLoader::LoadTextureFromFile("C:\\Dev\\FlyCore\\Assets\\skybox\\right.jpg", l_skybox_textures[1]);
	Utility::AssetLoader::LoadTextureFromFile("C:\\Dev\\FlyCore\\Assets\\skybox\\top.jpg", l_skybox_textures[2]);
	Utility::AssetLoader::LoadTextureFromFile("C:\\Dev\\FlyCore\\Assets\\skybox\\bottom.jpg", l_skybox_textures[3]);
	Utility::AssetLoader::LoadTextureFromFile("C:\\Dev\\FlyCore\\Assets\\skybox\\back.jpg", l_skybox_textures[4]);
	Utility::AssetLoader::LoadTextureFromFile("C:\\Dev\\FlyCore\\Assets\\skybox\\front.jpg", l_skybox_textures[5]);
	m_skyBox = new D3D12TextureCube(l_skybox_textures[0].width, l_skybox_textures[0].height);
	m_uploadBuffer->CopyData(l_skybox_textures[0].data, l_skybox_textures[0].size);
	m_uploadBuffer->CopyData(l_skybox_textures[1].data, l_skybox_textures[1].size);
	m_uploadBuffer->CopyData(l_skybox_textures[2].data, l_skybox_textures[2].size);
	m_uploadBuffer->CopyData(l_skybox_textures[3].data, l_skybox_textures[3].size);
	m_uploadBuffer->CopyData(l_skybox_textures[4].data, l_skybox_textures[4].size);
	m_uploadBuffer->CopyData(l_skybox_textures[5].data, l_skybox_textures[5].size);
	//m_uploadBuffer->CopyData(l_texture.data, l_texture.size);
	//m_uploadBuffer->CopyData(l_texture.data, l_texture.size);
	//m_uploadBuffer->CopyData(l_texture.data, l_texture.size);
	//m_uploadBuffer->CopyData(l_texture.data, l_texture.size);
	//m_uploadBuffer->CopyData(l_texture.data, l_texture.size);
	//m_uploadBuffer->CopyData(l_texture.data, l_texture.size);

	m_skyBox->SetName(L"Skybox Texture");


	l_graphicsContext.Begin(nullptr);
	l_graphicsContext.CopyTextureCubeData(m_skyBox, m_uploadBuffer->GetResource());
	//l_graphicsContext.CopyTextureCubeData(
	//	m_skyBox,
	//	m_uploadBuffer->GetResource(),
	//	l_texture.width, l_texture.height, DXGI_FORMAT_R8G8B8A8_UNORM);
	l_graphicsContext.TransitResourceState(m_skyBox->GetResource(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	l_graphicsContext.End(true);

}

void Renderer::D3D12Renderer::LoadUI(UI::UISystem* p_system)
{

	auto& g_descManager = D3D12DescManager::GetDescManager();
	auto l_fontDescHandle = g_descManager.RequestDesc(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_ui = static_cast<UI::D3D12UISystem*>(p_system);
	m_ui->Init(m_device,
		Constants::SWAPCHAIN_BUFFER_COUNT,
		Constants::SwapChainFormat,
		g_descManager.GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)->GetHeap(),
		l_fontDescHandle->cpuHandle, 
		l_fontDescHandle->gpuHandle);

}

void Renderer::D3D12Renderer::InitPipelineState()
{
   
    using namespace Microsoft::WRL;

    // Create the pipeline state, which includes compiling and loading shaders.
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
	ComPtr<ID3DBlob> quadShader_vs;
	ComPtr<ID3DBlob> quadShader_ps;
	ComPtr<ID3DBlob> shadow_pass_vs;
	ComPtr<ID3DBlob> shadow_pass_ps;
	ComPtr<ID3DBlob> skybox_vs;
	ComPtr<ID3DBlob> skybox_ps;


#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    //UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

	UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#else
    UINT compileFlags = 0;
#endif

    D3DCompileFromFile(L"C:\\Dev\\FlyCore\\Renderer\\forward_vs.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
    D3DCompileFromFile(L"C:\\Dev\\FlyCore\\Renderer\\forward_ps.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);
	D3DCompileFromFile(L"C:\\Dev\\FlyCore\\Renderer\\frame_quad_vs.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0, &quadShader_vs, nullptr);
	D3DCompileFromFile(L"C:\\Dev\\FlyCore\\Renderer\\frame_quad_ps.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", compileFlags, 0, &quadShader_ps, nullptr);
	D3DCompileFromFile(L"C:\\Dev\\FlyCore\\Renderer\\shadow_pass_vs.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0, &shadow_pass_vs, nullptr);
	D3DCompileFromFile(L"C:\\Dev\\FlyCore\\Renderer\\shadow_pass_ps.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", compileFlags, 0, &shadow_pass_ps, nullptr);
	D3DCompileFromFile(L"C:\\Dev\\FlyCore\\Renderer\\skybox_vs.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0, &skybox_vs, nullptr);
	D3DCompileFromFile(L"C:\\Dev\\FlyCore\\Renderer\\skybox_ps.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", compileFlags, 0, &skybox_ps, nullptr);

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 52, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }

	};

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_clusterForwardRootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState= CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 3;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R10G10B10A2_UNORM;
	psoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	psoDesc.RTVFormats[2] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_graphicsPipelineState));

	psoDesc.VS = CD3DX12_SHADER_BYTECODE(quadShader_vs.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(quadShader_ps.Get());
	psoDesc.pRootSignature = m_finalOutputRootSignature;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
	psoDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
	psoDesc.DepthStencilState.DepthEnable = false;
	m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_quadPipelineState));

	psoDesc.VS = CD3DX12_SHADER_BYTECODE(skybox_vs.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(skybox_ps.Get());
	psoDesc.pRootSignature = m_skyboxRootSignature;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
	psoDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_skyboxPipelineState));


	psoDesc.VS = CD3DX12_SHADER_BYTECODE(shadow_pass_vs.Get());
	psoDesc.PS = {};
	psoDesc.pRootSignature = m_shadowPassRootSignature;
	psoDesc.NumRenderTargets = 0;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	psoDesc.RTVFormats[1] = DXGI_FORMAT_UNKNOWN;
	psoDesc.RTVFormats[2] = DXGI_FORMAT_UNKNOWN;
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.RasterizerState.DepthBias = 10;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.RasterizerState.SlopeScaledDepthBias =20.25;
	m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_shadowPassPipelineState));


	//Compute Pipieline state
	{
		ComPtr<ID3DBlob> computeShader;
		D3DCompileFromFile(L"C:\\Dev\\FlyCore\\Renderer\\lightcull_cs.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "cs_5_0", compileFlags, 0, &computeShader, nullptr);
		D3D12_COMPUTE_PIPELINE_STATE_DESC l_computePipelineStateDesc = {};
		l_computePipelineStateDesc.pRootSignature = m_lightCullRootSignature;
		l_computePipelineStateDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader.Get());
		m_device->CreateComputePipelineState(&l_computePipelineStateDesc, MY_IID_PPV_ARGS(&m_lightCullPipelineState));
	
		ComPtr<ID3DBlob> ssr_computeShader;
		D3DCompileFromFile(L"C:\\Dev\\FlyCore\\Renderer\\ssr_cs.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "cs_5_0", compileFlags, 0, &ssr_computeShader, nullptr);
		D3D12_COMPUTE_PIPELINE_STATE_DESC l_ssr_computeStateDesc = {};
		l_ssr_computeStateDesc.CS = CD3DX12_SHADER_BYTECODE(ssr_computeShader.Get());
		l_ssr_computeStateDesc.pRootSignature = m_ssrRootSignature;
		m_device->CreateComputePipelineState(&l_ssr_computeStateDesc, MY_IID_PPV_ARGS(&m_ssrPipelineState));
	}
}

void Renderer::D3D12Renderer::InitRenderpass()
{
	auto& l_graphicsContext = D3D12GraphicsContext::GetContext();
	
	D3D12_RENDER_PASS_RENDER_TARGET_DESC l_lightRT = {};
	l_lightRT.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
	l_lightRT.BeginningAccess.Clear.ClearValue.Color[0] = Constants::CLEAR_COLOR[0];
	l_lightRT.BeginningAccess.Clear.ClearValue.Color[1] = Constants::CLEAR_COLOR[1];
	l_lightRT.BeginningAccess.Clear.ClearValue.Color[2] = Constants::CLEAR_COLOR[2];
	l_lightRT.BeginningAccess.Clear.ClearValue.Color[3] = Constants::CLEAR_COLOR[3];
	l_lightRT.BeginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
	l_lightRT.cpuDescriptor = l_graphicsContext.GetRenderTarget("Light")->GetRTV()->cpuHandle;
	l_lightRT.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

	D3D12_RENDER_PASS_RENDER_TARGET_DESC l_normalRT = {};
	l_normalRT.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
	l_normalRT.BeginningAccess.Clear.ClearValue.Color[0] = Constants::CLEAR_COLOR[0];
	l_normalRT.BeginningAccess.Clear.ClearValue.Color[1] = Constants::CLEAR_COLOR[1];
	l_normalRT.BeginningAccess.Clear.ClearValue.Color[2] = Constants::CLEAR_COLOR[2];
	l_normalRT.BeginningAccess.Clear.ClearValue.Color[3] = Constants::CLEAR_COLOR[3];
	l_normalRT.BeginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	l_normalRT.cpuDescriptor = l_graphicsContext.GetRenderTarget("Normal")->GetRTV()->cpuHandle;
	l_normalRT.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

	D3D12_RENDER_PASS_RENDER_TARGET_DESC l_specularRT = {};
	l_specularRT.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
	l_specularRT.BeginningAccess.Clear.ClearValue.Color[0] = Constants::CLEAR_COLOR[0];
	l_specularRT.BeginningAccess.Clear.ClearValue.Color[1] = Constants::CLEAR_COLOR[1];
	l_specularRT.BeginningAccess.Clear.ClearValue.Color[2] = Constants::CLEAR_COLOR[2];
	l_specularRT.BeginningAccess.Clear.ClearValue.Color[3] = Constants::CLEAR_COLOR[3];
	l_specularRT.BeginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	l_specularRT.cpuDescriptor = l_graphicsContext.GetRenderTarget("Specular")->GetRTV()->cpuHandle;
	l_specularRT.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

	D3D12_RENDER_PASS_DEPTH_STENCIL_DESC l_depthRT = {};
	l_depthRT.cpuDescriptor = l_graphicsContext.GetDepthBuffer("DepthBuffer")->GetDSV()->cpuHandle;
	l_depthRT.DepthBeginningAccess.Clear.ClearValue.DepthStencil = { 1.0f,0 };
	l_depthRT.DepthBeginningAccess.Clear.ClearValue.Format = Constants::DepthFormat;
	l_depthRT.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
	//l_depthRT.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

	m_clusterForwardPass.mrt = { l_lightRT ,l_normalRT ,l_specularRT };
	m_clusterForwardPass.depth = l_depthRT;
}

void Renderer::D3D12Renderer::InitBuiltinMeshes()
{
	using namespace Gameplay;

	std::vector<Vertex> l_vertices = {
		{ {-1.0f, 1.0f,0.0f,1.0},{0.0f,0.0f,0.0},{0.0f,0.0f}} ,
		{ {-1.0f,-1.0f,0.0f,1.0},{0.0f,0.0f,0.0},{0.0f,1.0f}} ,
		{ { 1.0f,-1.0f,0.0f,1.0},{0.0f,0.0f,0.0},{1.0f,1.0f}} ,
		{ { 1.0f, 1.0f,0.0f,1.0},{0.0f,0.0f,0.0},{1.0f,0.0f}} ,

	};
	std::vector<uint32_t> l_indices =
	{
		0,2,1,
		0,3,2
	};
	//-1 1  0,0         1  1  1,0
	//-1-1	0,1	   	    1 -1  1,1
	std::vector<Vertex> l_skybox_vertices = {
		// positions      
		{{-1.0f,  1.0f, 1.0f,  1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}},
		{{-1.0f, -1.0f, 1.0f,  1.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f}},
		{{1.0f,  -1.0f, 1.0f,  1.0f},{0.0f,0.0f,0.0f},{1.0f,1.0f}},
		{{1.0f,   1.0f, 1.0f,  1.0f},{0.0f,0.0f,0.0f},{1.0f,0.0f}},

		{{-1.0f,  1.0f, -1.0f,  1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}},
		{{-1.0f, -1.0f, -1.0f,  1.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f}},
		{{1.0f,  -1.0f, -1.0f,  1.0f},{0.0f,0.0f,0.0f},{1.0f,1.0f}},
		{{1.0f,   1.0f, -1.0f,  1.0f},{0.0f,0.0f,0.0f},{1.0f,0.0f}},

		{{-1.0f, 1.0f, 1.0f,   1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}},
		{{-1.0f, 1.0f,-1.0f,   1.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f}},
		{{1.0f,  1.0f,-1.0f,   1.0f},{0.0f,0.0f,0.0f},{1.0f,1.0f}},
		{{1.0f,  1.0f, 1.0f,   1.0f},{0.0f,0.0f,0.0f},{1.0f,0.0f}},

		{{-1.0f, -1.0f, 1.0f,   1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}},
		{{-1.0f, -1.0f,-1.0f,   1.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f}},
		{{1.0f,  -1.0f,-1.0f,   1.0f},{0.0f,0.0f,0.0f},{1.0f,1.0f}},
		{{1.0f,  -1.0f, 1.0f,   1.0f},{0.0f,0.0f,0.0f},{1.0f,0.0f}},

		{{1.0f,-1.0f,  1.0f,   1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}},
		{{1.0f,-1.0f, -1.0f,   1.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f}},
		{{1.0f,1.0f,  -1.0f,   1.0f},{0.0f,0.0f,0.0f},{1.0f,1.0f}},
		{{1.0f,1.0f,   1.0f,   1.0f},{0.0f,0.0f,0.0f},{1.0f,0.0f}},

		{{-1.0f,-1.0f,  1.0f,   1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f}},
		{{-1.0f,-1.0f, -1.0f,   1.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f}},
		{{-1.0f,1.0f,  -1.0f,   1.0f},{0.0f,0.0f,0.0f},{1.0f,1.0f}},
		{{-1.0f,1.0f,   1.0f,   1.0f},{0.0f,0.0f,0.0f},{1.0f,0.0f}},
	};

	std::vector<uint32_t> l_skybox_indices =
	{
		0 + 0 * 4,1 + 0 * 4,2 + 0 * 4,0 + 0 * 4,2 + 0 * 4,3 + 0 * 4,
		0 + 1 * 4,1 + 1 * 4,2 + 1 * 4,0 + 1 * 4,2 + 1 * 4,3 + 1 * 4,
		0 + 2 * 4,1 + 2 * 4,2 + 2 * 4,0 + 2 * 4,2 + 2 * 4,3 + 2 * 4,
		0 + 3 * 4,1 + 3 * 4,2 + 3 * 4,0 + 3 * 4,2 + 3 * 4,3 + 3 * 4,
		0 + 4 * 4,1 + 4 * 4,2 + 4 * 4,0 + 4 * 4,2 + 4 * 4,3 + 4 * 4,
		0 + 5 * 4,1 + 5 * 4,2 + 5 * 4,0 + 5 * 4,2 + 5 * 4,3 + 5 * 4,

	};
	m_frameQuad.m_quadMesh = Mesh(std::move(l_vertices), std::move(l_indices));

	m_skyBoxMesh.m_quadMesh = Mesh(std::move(l_skybox_vertices), std::move(l_skybox_indices));

	m_frameQuad.m_quadVertexOffset = m_vertexOffsetInByte / sizeof(Vertex);
	m_uploadBuffer->CopyData(l_vertices.data(), sizeof(Vertex) * l_vertices.size());
	m_vertexOffsetInByte += static_cast<uint32_t>(sizeof(Vertex) * l_vertices.size());

	m_skyBoxMesh.m_quadVertexOffset = m_vertexOffsetInByte / sizeof(Vertex);
	m_uploadBuffer->CopyData(l_skybox_vertices.data(), sizeof(Vertex) * l_skybox_vertices.size());
	m_vertexOffsetInByte += static_cast<uint32_t>(sizeof(Vertex) * l_skybox_vertices.size());


	m_frameQuad.m_quadIndexOffset = m_indexOffsetInByte / sizeof(uint32_t);
	m_uploadBuffer->CopyData(l_indices.data(), sizeof(uint32_t) * l_indices.size());
	m_indexOffsetInByte += static_cast<uint32_t>(sizeof(uint32_t) * l_indices.size());

	m_skyBoxMesh.m_quadIndexOffset = m_indexOffsetInByte / sizeof(uint32_t);
	m_uploadBuffer->CopyData(l_skybox_indices.data(), sizeof(uint32_t) * l_skybox_indices.size());
	m_indexOffsetInByte += static_cast<uint32_t>(sizeof(uint32_t) * l_skybox_indices.size());

	auto& l_graphicsContext = D3D12GraphicsCmdContext::GetContext();
	l_graphicsContext.Begin(nullptr);
	l_graphicsContext.UploadVertexBuffer(m_vertexBuffer, 0, m_uploadBuffer, 0, m_vertexOffsetInByte);
	l_graphicsContext.UploadVertexBuffer(m_indexBuffer, 0, m_uploadBuffer, m_vertexOffsetInByte, m_indexOffsetInByte);
	l_graphicsContext.TransitResourceState(m_vertexBuffer->GetResource(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	l_graphicsContext.TransitResourceState(m_indexBuffer->GetResource(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_INDEX_BUFFER);
	l_graphicsContext.End(true);
	m_uploadBuffer->ResetBuffer();

}

void Renderer::D3D12Renderer::OnDestory()
{
	SAFE_DELETE(m_VSUniform);
	SAFE_DELETE(m_PSUniform);
    SAFE_DELETE(m_lightList);
	SAFE_DELETE(m_renderCmdQueue);
	SAFE_DELETE(m_lightCullCmd);
	SAFE_DELETE(m_graphicsCmd);
    SAFE_DELETE(m_vertexBuffer); 
    SAFE_DELETE(m_indexBuffer);
    SAFE_DELETE(m_uploadBuffer);
	SAFE_DELETE(m_lightBuffer);
	SAFE_DELETE(m_mainCamera);
	SAFE_DELETE(m_defaultTexture);
	SAFE_DELETE(m_skyBox);
}


