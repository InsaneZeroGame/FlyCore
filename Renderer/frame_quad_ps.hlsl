#include "shader_common.hlsli"

Texture2D<float4> LightBuffer : register(t0);
Texture2D<float4> Normal :	register(t1);
Texture2D<float4> Specular: register(t2);
SamplerState defaultSampler : register(s0);

float4 main(PSInput input) : SV_TARGET
{
	return float4(1.0f,1.0f,0.0f,1.0f);
	return LightBuffer.Sample(defaultSampler,input.uv);
}