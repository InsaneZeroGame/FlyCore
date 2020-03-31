#include "shader_common.hlsli"



ShadowOut main(float4 position : POSITION, float3 normal : NORMAL0, float2 tex_uv : TEXCOORD0)
{
	ShadowOut res;
	res.position = mul(shadowMatrix,position);
	return res;
}