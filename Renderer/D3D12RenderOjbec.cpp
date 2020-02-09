#include "D3D12RenderOjbec.h"

Renderer::D3D12RenderObject::D3D12RenderObject(
	uint32_t IndexCountPerInstance,
	uint32_t InstanceCount,
	uint32_t StartIndexLocation,
	int32_t  BaseVertexLocation,
	uint32_t StartInstanceLocation
):
	IndexCountPerInstance(IndexCountPerInstance),
	InstanceCount(InstanceCount),
	StartIndexLocation(StartIndexLocation),
	BaseVertexLocation(BaseVertexLocation),
	StartInstanceLocation(StartInstanceLocation)
{
}

Renderer::D3D12RenderObject::~D3D12RenderObject()
{
}
