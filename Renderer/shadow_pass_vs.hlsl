#include "shader_common.hlsli"



ShadowOut main(float4 position : POSITION, float3 normal : NORMAL0, float2 tex_uv : TEXCOORD0 ,int4 boneIndex: BONEINDEX,float4 boneWeight : BONEWEIGHT)
{
	ShadowOut res;
	res.position = mul(shadowMatrix,mul(model,position));
	return res;
}