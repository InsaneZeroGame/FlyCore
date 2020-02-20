#pragma once
#include <d3d12.h>
#if defined(NTDDI_WIN10_RS2) && (NTDDI_VERSION >= NTDDI_WIN10_RS2)
#include <dxgi1_6.h>
#else
#include <dxgi1_4.h>    // For WARP
#endif
#include "D3D12Common.h"


namespace Renderer
{
	class D3D12Device : public Interface::INoCopy
	{
	public:
		static D3D12Device& GetDevice()
		{
			static D3D12Device l_device;
			return l_device;
		}

		~D3D12Device();

		operator ID3D12Device* () { return m_device; }

	private:
		D3D12Device();

		ID3D12Device* m_device = nullptr;


	};
}