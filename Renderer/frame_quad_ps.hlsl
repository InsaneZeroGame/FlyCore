#include "shader_common.hlsli"

Texture2D<float4> LightBuffer : register(t0);
Texture2D<float4> Normal :	register(t1);
Texture2D<float4> Specular: register(t2);
SamplerState defaultSampler : register(s0);

#define MAX_RAY_LENGTH 40


float4 main(SSRPSinput input) : SV_TARGET
{
	//return float4(1.0f,1.0f,0.0f,1.0f);
	return float4(LightBuffer.Sample(defaultSampler,input.uv));

	float zFar = zNearFar.y;
	float zNear = zNearFar.x;
	float viewSpaceDepth = Specular.Sample(defaultSampler,input.uv).a;
	float linearDepth = 1.0f / (viewSpaceDepth * (zFar - zNear) / zNear + 1.0f);
	float3 viewSpaceNormal = Normal.Sample(defaultSampler, input.uv).xyz;
	float3 rayOrigin = linearDepth * input.cameraRay.xyz;
	float3 rayDirection = normalize(reflect(input.cameraRay,viewSpaceNormal));

	float rayLength = rayOrigin.z + rayDirection.z * MAX_RAY_LENGTH < zNear ?
		(zNear - rayOrigin.z) / rayDirection.z : MAX_RAY_LENGTH;

	float3 rayEnd = rayOrigin + rayDirection * rayLength;


	//Projects to clip space
	//float4 rayOriginClip = mul(project, float4(rayOrigin, 1.0f));
	//float4 rayEndClip = mul(project, float4(rayEnd, 1.0f));
	//float4 rayOriginClip /= rayOriginClip.w;
	//float4 rayEndClip /= rayEndClip.w;
	//
	////Math that ensure ray march at least move one pixel.
	//float2 rayOriginXY = rayOriginClip.xy;
	//float2 rayEndXY = rayEndClip.xy;
	//rayEndXY += dot(rayOriginXY - rayEndXY , rayOriginXY - rayEndXY) < 0.0001 ? 0.01:0.0;
	//float2 delta = rayEndXY - rayOriginXY;
	//
	//bool premute = false;
	//if (abs(delta.x) < abs(delta.y))
	//{
	//	delta = delta.yx;
	//	rayOriginXY = rayOrigin.yx;
	//	rayEndXY = rayEndXY.yx;
	//
	//}
	//
	//float stepDir = sign(delta.x);
	//float invertDX = stepDir / delta.x;

	float2 hitUV;
	float3 hitPos;
	float hitCount;
	float jitter;
	//bool hitTest = traceScreenSpaceRay(rayOrigin, rayEnd,jitter,hitUV,hitPos,jitter);

}