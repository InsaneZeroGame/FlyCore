#pragma once

#define MY_IID_PPV_ARGS IID_PPV_ARGS
#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)


namespace Renderer
{

	namespace Constants
	{
		static const uint32_t COMPUTE_CMD_COUNT = 1;
		static const uint64_t MAX_CONST_BUFFER_VIEW_SIZE = 65536;
		static const uint64_t VERTEX_BUFFER_SIZE = 10 * 1024 * 1024;
		static const uint32_t SWAPCHAIN_BUFFER_COUNT = 3;
		static DXGI_FORMAT SwapChainFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
		static const float CLEAR_COLOR[] = { 0.5F, 0.5F, 0.5F, 1.0f };
		static bool TypedUAVLoadSupport_R11G11B10_FLOAT = false;
		static bool TypedUAVLoadSupport_R16G16B16A16_FLOAT = false;
		static const uint32_t WORK_GROUP_SIZE_X = 16;
		static const uint32_t WORK_GROUP_SIZE_Y = 8;
		static const uint32_t WORK_GROUP_SIZE_Z = 8;
		static const uint32_t MAX_NUM_LIGHT_COUNT_PER_TILE = 32;
	};
	struct D3D12Descriptor
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		D3D12_DESCRIPTOR_HEAP_TYPE type;
	};
}