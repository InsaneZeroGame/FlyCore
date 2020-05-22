#include "D3D12UI.h"
UI::D3D12UISystem::D3D12UISystem(Framework::Window* p_window):
	UISystem(p_window)
{
    
}

UI::D3D12UISystem::~D3D12UISystem()
{
}

void UI::D3D12UISystem::Init(ID3D12Device* device, int num_frames_in_flight, DXGI_FORMAT rtv_format, ID3D12DescriptorHeap* cbv_srv_heap, D3D12_CPU_DESCRIPTOR_HANDLE font_srv_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE font_srv_gpu_desc_handle)
{
    ImGui_ImplDX12_Init(device, num_frames_in_flight,
        rtv_format, cbv_srv_heap,
        font_srv_cpu_desc_handle,
        font_srv_gpu_desc_handle);
}
