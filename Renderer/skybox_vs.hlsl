#include "shader_common.hlsli"



SkyboxPSInput main(float4 position : POSITION, float3 normal : NORMAL0, float2 tex_uv : TEXCOORD0)
{
	SkyboxPSInput res;
	float4x4 viewMatrix = view;
	viewMatrix._m03_m13_m23_m33 = float4(0.0, 0.0, 0.0, 1.0);
	viewMatrix._m30_m31_m32_m33 = float4(0.0, 0.0, 0.0, 1.0);
	res.position = mul(project,mul(viewMatrix,position)).xyww;
	res.uv = position;
	return res;
}