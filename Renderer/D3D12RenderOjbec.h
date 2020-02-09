#pragma once
#include "stdafx.h"
#include "../Framework/INoCopy.h"

namespace Renderer
{
	struct D3D12RenderObject: public Interface::INoCopy
	{
	public:
		D3D12RenderObject(
		uint32_t IndexCountPerInstance,
		uint32_t InstanceCount,
		uint32_t StartIndexLocation,
		int32_t  BaseVertexLocation,
		uint32_t StartInstanceLocation
		);
		~D3D12RenderObject();
		const uint32_t IndexCountPerInstance;
		const uint32_t InstanceCount;
		const uint32_t StartIndexLocation;
		const int32_t  BaseVertexLocation;
		const uint32_t StartInstanceLocation;

	};
}