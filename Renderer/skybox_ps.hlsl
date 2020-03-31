#include "shader_common.hlsli"

TextureCube Skybox : register(t0);
SamplerState CubeSampler : register(s0);

float4 main(SkyboxPSInput input) : SV_TARGET
{
	//return float4(input.uv,0.0f,1.0);
	return Skybox.SampleLevel(CubeSampler,float3(input.uv.xyz),0) + 0.15;
}