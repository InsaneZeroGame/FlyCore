#pragma once
#include "stdafx.h"
#include "../Framework/UI.h"
#include "imgui_impl_dx12.h"

namespace UI
{
	class D3D12UISystem final: public UISystem
	{
	public:
		D3D12UISystem(Framework::Window* p_window);
		~D3D12UISystem();
		void Init(ID3D12Device* device, int num_frames_in_flight, DXGI_FORMAT rtv_format, ID3D12DescriptorHeap* cbv_srv_heap,
			D3D12_CPU_DESCRIPTOR_HANDLE font_srv_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE font_srv_gpu_desc_handle);

	private:

	};

}