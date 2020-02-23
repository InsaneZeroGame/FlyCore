#include "stdafx.h"
#include "D3D12Renderer.h"
#include <D3Dcompiler.h>
#include "../Utility/AssetLoader.h"


#define STB_IMAGE_IMPLEMENTATION
#include "../3dparty/include/stb_image.h"

#define CAMERA_UNIFORM_ROOT_INDEX 0
#define LIGHT_UAV_ROOT_INDEX 1


Renderer::D3D12Renderer::D3D12Renderer():
    m_device(D3D12Device::GetDevice()),
    m_swapChain(nullptr),
    m_frameIndex(0),
	m_graphicsCmd(new D3D12GraphicsCmd(Constants::SWAPCHAIN_BUFFER_COUNT)),
	m_computeCmd(new D3D12GraphicsCmd(Constants::COMPUTE_CMD_COUNT)),
	m_VSUniform(nullptr),
	m_renderCmdQueue(new D3D12CmdQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)),
    m_lightBuffer(nullptr)
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
    InitBuffers();
    InitRootSignature();
    InitPipelineState();
}

void Renderer::D3D12Renderer::OnUpdate()
{
	auto& l_graphicsContext = D3D12GraphicsContext::GetContext();
    m_graphicsCmd->Reset(m_frameIndex, m_graphicsPipelineState);
    // Indicate that the back buffer will be used as a render target.
    l_graphicsContext.BeginRender(m_frameIndex);
	//Light cull 
	{
		using namespace Constants;
		m_computeCmd->Reset(0, m_computePipelineState);
		ID3D12GraphicsCommandList* l_computeCmdList = *m_computeCmd;
		l_computeCmdList->SetPipelineState(m_computePipelineState);
		l_computeCmdList->SetComputeRootSignature(m_computeRootSignature);
		l_computeCmdList->SetComputeRootConstantBufferView(CAMERA_UNIFORM_ROOT_INDEX, m_VSUniform->GetGpuVirtualAddress());
        l_computeCmdList->SetComputeRootUnorderedAccessView(LIGHT_UAV_ROOT_INDEX, m_lightBuffer->GetGpuVirtualAddress());
        l_computeCmdList->Dispatch(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y, WORK_GROUP_SIZE_Z);
		m_computeCmd->Close();
		m_computeCmd->Flush(true);
	}
	
	
	ID3D12GraphicsCommandList* l_graphicsCmdList = *m_graphicsCmd;
	//Forward Pass
	{
		l_graphicsCmdList->SetPipelineState(m_graphicsPipelineState);
		// Set necessary state.
		l_graphicsCmdList->SetGraphicsRootSignature(m_graphicsRootSignature);
		l_graphicsCmdList->IASetIndexBuffer(&m_indexBuffer->GetIndexBufferView());
		l_graphicsCmdList->IASetVertexBuffers(0, 1, &m_vertexBuffer->GetVertexBufferView());
		l_graphicsCmdList->SetGraphicsRootConstantBufferView(CAMERA_UNIFORM_ROOT_INDEX, m_VSUniform->GetGpuVirtualAddress());
		l_graphicsCmdList->SetGraphicsRootShaderResourceView(LIGHT_UAV_ROOT_INDEX, m_lightBuffer->GetGpuVirtualAddress());
		l_graphicsCmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		for (auto& l_actor = m_scene->m_actors.begin(); l_actor < m_scene->m_actors.end(); ++l_actor)
		{
			for (auto& l_mesh = l_actor->m_meshes.begin(); l_mesh < l_actor->m_meshes.end(); l_mesh++)
			{
				l_graphicsCmdList->DrawIndexedInstanced(static_cast<uint32_t>(l_mesh->m_indices.size()), 1, l_mesh->m_indexOffset, l_mesh->m_vertexOffset, 0);
			}
		}
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
	D3D12GraphicsContext::GetContext().InitRenderTargets(m_window->GetWidth(), m_window->GetHeight(), m_swapChain.Get());
	D3D12GraphicsContext::GetContext().SetGraphicsCmdList(*m_graphicsCmd);

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

static float RandomFloat01()
{
	//return 0.0-1.0
	return float(rand()) / float(RAND_MAX);
}

static float RandomFloat_11()
{
	//return -1.0-1.0
	return 2 * (RandomFloat01() - 0.5f);
}

void Renderer::D3D12Renderer::InitBuffers()
{
    auto& loader = Utility::AssetLoader::GetLoader();
    
    m_scene = new Renderer::Scene;
    loader.LoadFbx("D:\\Dev\\FlyCore\\build\\Game\\Debug\\scene.fbx", m_scene);

    m_vertexBuffer = new D3D12VertexBuffer(Constants::VERTEX_BUFFER_SIZE);
    m_uploadBuffer = new D3D12UploadBuffer(Constants::MAX_CONST_BUFFER_VIEW_SIZE);
    m_indexBuffer = new D3D12IndexBuffer(Constants::VERTEX_BUFFER_SIZE);
	m_VSUniform = new D3D12UploadBuffer(Utility::AlignTo256(sizeof(SceneUniformBuffer)));
	
	
	m_uniformBuffer.zNearFar[1] = 55.0f;
	m_uniformBuffer.zNearFar[0] = 0.01f;

	m_uniformBuffer.m_proj = glm::perspectiveFovLH(45.0f, 15.0f, 15.0f, m_uniformBuffer.zNearFar[0], m_uniformBuffer.zNearFar[1]);
	m_uniformBuffer.m_inverProj = glm::inverse(m_uniformBuffer.m_proj);
	m_uniformBuffer.m_view = glm::lookAtLH(glm::vec3(10, 20.0, 10), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	
	for (auto i = 0; i < m_uniformBuffer.m_lights.size(); ++i)
	{
		m_uniformBuffer.m_lights[i] = PointLight();
		m_uniformBuffer.m_lights[i].isActive = false;
	}

	float step = 40.0f / 16;

	for (auto i = 0; i < 16; ++i)
	{
		for (auto j = 0; j < 16; ++j)
		{
			m_uniformBuffer.m_lights[j * 16 + i].isActive = true;

			auto lightPosView = m_uniformBuffer.m_view * glm::vec4(0.0,0.5,0.0,1.0f);
			//auto lightPosView =  glm::vec4(0.0, 0.5, 0.0, 1.0f);

			m_uniformBuffer.m_lights[j * 16 + i].position[0] = lightPosView.x;
			m_uniformBuffer.m_lights[j * 16 + i].position[1] = lightPosView.y;
			m_uniformBuffer.m_lights[j * 16 + i].position[2] = lightPosView.z;
			m_uniformBuffer.m_lights[j * 16 + i].position[3] = lightPosView.w;
			m_uniformBuffer.m_lights[j * 16 + i].color[0] = RandomFloat01();
			m_uniformBuffer.m_lights[j * 16 + i].color[1] = RandomFloat01();
			m_uniformBuffer.m_lights[j * 16 + i].color[2] = RandomFloat01();
			m_uniformBuffer.m_lights[j * 16 + i].color[3] = RandomFloat01();
			m_uniformBuffer.m_lights[j * 16 + i].radius = 1;
		}
	}

	m_VSUniform->CopyData(&m_uniformBuffer, sizeof(SceneUniformBuffer));

	UINT vertexBufferSize = 0;
	for (auto i = 0; i < m_scene->m_actors.size(); ++i)
	{
	   for (auto j = 0; j < m_scene->m_actors[i].m_meshes.size(); ++j)
	   {
		   auto l_vertexSize = (UINT)sizeof(m_scene->m_actors[i].m_meshes[j].m_vertices[0]) * static_cast<uint32_t>(m_scene->m_actors[i].m_meshes[j].m_vertices.size());
		   m_uploadBuffer->CopyData(m_scene->m_actors[i].m_meshes[j].m_vertices.data(), l_vertexSize);
		   vertexBufferSize += l_vertexSize;
	   }
	}

	UINT indexBufferSize = 0;
	for (auto i = 0; i < m_scene->m_actors.size(); ++i)
	{
	   for (auto j = 0; j < m_scene->m_actors[i].m_meshes.size(); j++)
	   {
		   auto l_indexSize = (UINT)sizeof(m_scene->m_actors[i].m_meshes[j].m_indices[0]) * static_cast<uint32_t>(m_scene->m_actors[i].m_meshes[j].m_indices.size());
		   m_uploadBuffer->CopyData(m_scene->m_actors[i].m_meshes[j].m_indices.data(),l_indexSize);
		   indexBufferSize += l_indexSize;
	   }
	}

    auto & l_graphicsContext = D3D12GraphicsCmdContext::GetContext();
    l_graphicsContext.Begin(nullptr);
    l_graphicsContext.UploadVertexBuffer(m_vertexBuffer, 0, m_uploadBuffer, 0, vertexBufferSize);
    l_graphicsContext.UploadVertexBuffer(m_indexBuffer, 0, m_uploadBuffer, vertexBufferSize, indexBufferSize);
    l_graphicsContext.TransitResourceState(m_vertexBuffer->GetResource(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    l_graphicsContext.TransitResourceState(m_indexBuffer->GetResource(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_INDEX_BUFFER);
    l_graphicsContext.End(true);

    m_lightBuffer = new D3D12StructBuffer(1024, sizeof(LightList));
}

void Renderer::D3D12Renderer::InitRootSignature()
{
    using namespace Microsoft::WRL;
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


		std::vector<D3D12_ROOT_PARAMETER> l_rootParameters =
		{
			l_cameraUniform,
			l_lightUAV
		};

		rootSignatureDesc.Init(static_cast<uint32_t>(l_rootParameters.size()), l_rootParameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_graphicsRootSignature));

	}
   
	// Create Compute RS
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

		std::vector<D3D12_ROOT_PARAMETER> l_rootParameters =
		{
			l_cameraUniform,
            l_lightUAV
		};

		l_computeRootSignatureDesc.Init(static_cast<uint32_t>(l_rootParameters.size()), l_rootParameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3D12SerializeRootSignature(&l_computeRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_computeRootSignature));
	}
}

void Renderer::D3D12Renderer::CreateDefaultTexture()
{
}

void Renderer::D3D12Renderer::InitPipelineState()
{
   
    using namespace Microsoft::WRL;

    // Create the pipeline state, which includes compiling and loading shaders.
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    D3DCompileFromFile(L"D:\\Dev\\FlyCore\\Renderer\\forward_vs.hlsl", nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
    D3DCompileFromFile(L"D:\\Dev\\FlyCore\\Renderer\\forward_ps.hlsl", nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }

    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_graphicsRootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState= CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R10G10B10A2_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_graphicsPipelineState));


	//Compute Pipieline state
	{
		ComPtr<ID3DBlob> computeShader;
		D3DCompileFromFile(L"D:\\Dev\\FlyCore\\Renderer\\lightcull_cs.hlsl", nullptr, nullptr, "main", "cs_5_0", compileFlags, 0, &computeShader, nullptr);
		D3D12_COMPUTE_PIPELINE_STATE_DESC l_computePipelineStateDesc = {};
		l_computePipelineStateDesc.pRootSignature = m_computeRootSignature;
		l_computePipelineStateDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader.Get());;
		m_device->CreateComputePipelineState(&l_computePipelineStateDesc, MY_IID_PPV_ARGS(&m_computePipelineState));
	}
}

void Renderer::D3D12Renderer::LoadScene(Renderer::Scene*)
{
}

void Renderer::D3D12Renderer::OnDestory()
{
	SAFE_DELETE(m_VSUniform);
    SAFE_DELETE(m_lightBuffer);
	SAFE_DELETE(m_renderCmdQueue);
	SAFE_DELETE(m_computeCmd);
	SAFE_DELETE(m_graphicsCmd);
    SAFE_DELETE(m_vertexBuffer); 
    SAFE_DELETE(m_indexBuffer);
    SAFE_DELETE(m_uploadBuffer);
}
