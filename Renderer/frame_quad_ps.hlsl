#include "shader_common.hlsli"

Texture2D<float4> LightBuffer : register(t0);
Texture2D<float4> Normal :	register(t1);
Texture2D<float4> Specular: register(t2);
Texture2D<float4> SSR: register(t3);
//Texture2D<float> DepthBuffer:register(t3);
SamplerState defaultSampler : register(s0);

#define MAX_RAY_LENGTH 10

float4 main(SSRPSinput input) : SV_TARGET
{
	//return float4(1.0f,1.0f,0.0f,1.0f);
	return float4(SSR.Sample(defaultSampler,input.uv));
	//return DepthBuffer.Sample(defaultSampler, input.uv).r;

	//float zFar = zNearFar.y;
	//float zNear = zNearFar.x;
	//float viewSpaceDepth = Specular.Sample(defaultSampler,input.uv).a;
	//float3 viewSpaceNormal = Normal.Sample(defaultSampler, input.uv).xyz;
	//float3 rayOrigin = viewSpaceDepth * input.cameraRay.xyz;
	//float3 rayDirection = normalize(reflect(input.cameraRay,viewSpaceNormal));


	
	
}