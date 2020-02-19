#pragma once

#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable:4238) // nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable:4324) // structure was padded due to __declspec(align())

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <d3d12.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define MY_IID_PPV_ARGS IID_PPV_ARGS
#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#include "d3dx12.h"

#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <vector>
#include <queue>
#include <memory>
#include <string>
#include <exception>
#include <array>
#include <wrl.h>
#include <ppltasks.h>
#include "../Framework/Utility.h"
#include "../Framework/INoCopy.h"

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
};



namespace Renderer
{
	struct D3D12Descriptor
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		D3D12_DESCRIPTOR_HEAP_TYPE type;
	};
}


