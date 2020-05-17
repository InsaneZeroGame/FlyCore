#include "shader_common.hlsli"



ShadowOut main(float4 position : POSITION, float3 normal : NORMAL0, float2 tex_uv : TEXCOORD0 ,int4 boneIndex: BONEINDEX,float4 boneWeight : BONEWEIGHT)
{
	ShadowOut res;
	float4x4 modelMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	if (boneIndex.x != -1)
	{
		modelMatrix = boneMatrix[boneIndex.x] * boneWeight.x;
		modelMatrix += boneMatrix[boneIndex.y] * boneWeight.y;
		modelMatrix += boneMatrix[boneIndex.z] * boneWeight.z;
		modelMatrix += boneMatrix[boneIndex.w] * boneWeight.w;
	}


	res.position = mul(shadowMatrix,mul(mul(model,modelMatrix),position));
	return res;
}