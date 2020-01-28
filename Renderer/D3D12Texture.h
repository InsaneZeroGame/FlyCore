#pragma once
#include "stdafx.h"
#include "D3D12Resource.h"

namespace Renderer
{
	class D3D12Texture :  public D3D12Resource
	{
	public:
		D3D12Texture(D3D12_RESOURCE_DESC* p_desc, D3D12_HEAP_FLAGS p_flag, ID3D12Resource* p_resource = nullptr);
		~D3D12Texture();

	protected:
		D3D12_RESOURCE_DESC* m_resourceDesc;
	};
}