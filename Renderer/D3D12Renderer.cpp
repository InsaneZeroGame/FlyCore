#include "D3D12Renderer.h"
#include "stdafx.h"

Renderer::D3D12Renderer::D3D12Renderer():
    m_device(D3D12Device::GetDevice()),
    m_cmdQueue(nullptr),
    m_swapChain(nullptr),
    m_cmdListManager(D3D12CmdListManager::GetManagerPtr()),
    m_cmdAllocatorPool(D3D12CmdAllocatorPool::GetPoolPtr()),
    m_frameIndex(0)
{
}

Renderer::D3D12Renderer::~D3D12Renderer()
{
}

void Renderer::D3D12Renderer::OnInit()
{
    InitCmdQueue();
    ASSERT(m_window, "A window must be set before create swapchain\n");
    ASSERT(m_cmdQueue, "A CmdQueue must be created before create swapchain\n");

    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 3;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    m_device->CreateDescriptorHeap(&rtvHeapDesc, MY_IID_PPV_ARGS(&m_rtvHeap));

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    InitCmdLists();
    InitSwapChain();
    InitSyncPrimitive();

}

void Renderer::D3D12Renderer::OnUpdate()
{
    m_graphicsCmdAllocator[m_frameIndex]->Reset();
    m_cmdListManager->ResetCmdList(m_graphicsCmdList, m_graphicsCmdAllocator[m_frameIndex], nullptr);
    // Indicate that the back buffer will be used as a render target.
    m_graphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex]->GetResource(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

    // Record commands.
    const float clearColor[] = { 1.0f, 0.2f, 0.4f, 1.0f };
    m_graphicsCmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Indicate that the back buffer will now be used to present.
    m_graphicsCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex]->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    m_graphicsCmdList->Close();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_graphicsCmdList };
    m_cmdQueue->GetQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    // Present the frame.
    m_swapChain->Present(1, 0);
    SyncFrame();

}

void Renderer::D3D12Renderer::OnDestory()
{
    if (m_cmdQueue)
    {
        delete m_cmdQueue;
        m_cmdQueue = nullptr;
    }
}

void Renderer::D3D12Renderer::RenderScene()
{
}

void Renderer::D3D12Renderer::InitCmdQueue()
{
    m_cmdQueue = new D3D12CmdQueue(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void Renderer::D3D12Renderer::InitCmdLists()
{
    //request for 3 allocators for render cmd. one for each buffer for triple buffer.
    
    for (auto i = 0; i < Constants::SWAPCHAIN_BUFFER_COUNT; ++i)
    {
        m_graphicsCmdAllocator[i] = m_cmdAllocatorPool->RequestAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT);
    }

    m_cmdListManager->AllocateCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT,nullptr,m_graphicsCmdAllocator[0], MY_IID_PPV_ARGS(&m_graphicsCmdList));
    m_cmdListManager->CloseCmdList(m_graphicsCmdList);
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
    dxgiFactory->CreateSwapChainForHwnd(m_cmdQueue->GetQueue(), m_window->GetWin32Window(), &swapChainDesc, nullptr, nullptr, &swapChain1);
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

    D3D12_RESOURCE_DESC l_swapChainDesc = {};
    l_swapChainDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    l_swapChainDesc.Alignment = 0;
    l_swapChainDesc.Width = swapChainDesc.Width;
    l_swapChainDesc.Height = swapChainDesc.Height;
    l_swapChainDesc.DepthOrArraySize = 1;
    l_swapChainDesc.MipLevels = 0;
    l_swapChainDesc.Format = Constants::SwapChainFormat;
    l_swapChainDesc.SampleDesc.Count = 1;
    l_swapChainDesc.SampleDesc.Quality = 0;
    l_swapChainDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    l_swapChainDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

    for (uint32_t i = 0; i < Constants::SWAPCHAIN_BUFFER_COUNT; ++i)
    {
        ID3D12Resource* DisplayPlane;
        ASSERT_SUCCEEDED(m_swapChain->GetBuffer(i, MY_IID_PPV_ARGS(&DisplayPlane)));
        m_renderTargets[i] = new D3D12Texture(&l_swapChainDesc, D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES, DisplayPlane);
        m_device->CreateRenderTargetView(m_renderTargets[i]->GetResource(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }
}

void Renderer::D3D12Renderer::InitSyncPrimitive()
{
    m_fenceValues[0] = 0;
    m_fenceValues[1] = 0;
    m_fenceValues[2] = 0;
    m_fenceEvent = CreateEvent(0, false, false, 0);
    ASSERT_SUCCEEDED(m_device->CreateFence(m_fenceValues[0], D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, MY_IID_PPV_ARGS(&m_fence)));
}

void Renderer::D3D12Renderer::SyncFrame()
{
    //1.Get Current Available Backbuffer
    m_cmdQueue->GetQueue()->Signal(m_fence, m_fenceValues[m_frameIndex]);

    auto currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    //2.Query if cmdlist to this buffer is completed.
    auto debugIndex = m_fence->GetCompletedValue();
    if (m_fence->GetCompletedValue() < m_fenceValues[currentBackBufferIndex])
    {
        m_fence->SetEventOnCompletion(m_fenceValues[currentBackBufferIndex], m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
    m_fenceValues[currentBackBufferIndex] = m_fenceValues[m_frameIndex] + 1;
    m_frameIndex = currentBackBufferIndex;


}
