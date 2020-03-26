#include "shader_common.hlsli"

float linstep(float min, float max, float v)
{
	return saturate((v - min) / (max - min));
}

StructuredBuffer<LightList> LightBuffer : register(t1);
StructuredBuffer<PointLight> PointLights : register(t2);
Texture2D <float4> Alebdo : register(t3);
Texture2D <float> ShadowMap : register(t4);
SamplerState DefaultSampler :register(s0);
SamplerComparisonState ShadowSampler :register(s1);

static uint2  SCREEN_DIMENSION = uint2(1920, 1080);
static float4 sliceColor[16] = {
	float4(0.0,0.0,0.0,1.0f),
	float4(0.1,0.1,0.1,1.0f),
	float4(0.2,0.2,0.2,1.0f),
	float4(0.3,0.3,0.3,1.0f),
	float4(0.4,0.4,0.4,1.0f),
	float4(0.5,0.5,0.5,1.0f),
	float4(0.6,0.6,0.6,1.0f),
	float4(0.7,0.7,0.7,1.0f),
	float4(0.8,0.8,0.8,1.0f),
	float4(0.3,0.3,0.3,1.0f),
	float4(0.4,0.4,0.4,1.0f),
	float4(0.5,0.5,0.5,1.0f),
	float4(0.6,0.6,0.6,1.0f),
	float4(0.7,0.7,0.7,1.0f),
	float4(0.8,0.8,0.8,1.0f),
	float4(0.0,0.0,0.0,1.0f),

};

struct MRT
{
	float4 LightOut : COLOR0;
	float4 NormalOut : COLOR1;
	float4 SpecularOut:COLOR2;
};


MRT main(PSInput input) : SV_TARGET
{
	float zNear = zNearFar.x;
	float zFar = zNearFar.y;
	float zPosition = (input.scenePositionView.z - zNear) / (zFar - zNear);
	//float zPosition = log(input.scenePositionView.z)
	float2 screenPosition = input.position.xy / SCREEN_DIMENSION.xy;
	uint3 clusterPosition = uint3(screenPosition * uint2(8,8), zPosition * 16);

	float4 res_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	uint ClusterIndex = clusterPosition.z * (8 * 8) + clusterPosition.y * 8 + clusterPosition.x;
	float4 diffuse = float4(0.0, 0.0, 0.0, 1.0);
	float4 spec = 0.0f;
	float4 colorStep = 1.0/256.0;

	float4 diffuseDebug = 0.0f;
	MRT l_res;

	for (uint i = 0; i < 256; ++i)
	{
		if (LightBuffer[ClusterIndex].isActive[i] == 1)
		{
		float3 lightDir = PointLights[i].pos.xyz - input.scenePositionView.xyz;
		float3 lightDirNormalized = normalize(lightDir);
		float3 viewDir = normalize(float3(0.0,0.0,0.0) - input.scenePositionView.xyz);
		float3 halfwarDir = normalize(lightDir + viewDir);

		float lightDistSq = dot(lightDir, lightDir);
		float invLightDist = rsqrt(lightDistSq);
		float attenuation = 1.0 / (1.0 + PointLights[i].attenutation * pow(lightDistSq, 2));
		diffuse += max(dot(input.normal, lightDirNormalized), 0.0) * float4(PointLights[i].color) * attenuation;
		spec += pow(max(dot(input.normal, halfwarDir), 0.0),3.5) * attenuation;
		diffuseDebug += colorStep;
		}
	}
	float2 shadowCoord;
	shadowCoord.x = input.shadowUV.x * 0.5 + 0.5;
	shadowCoord.y = -input.shadowUV.y * 0.5 + 0.5;
	float shadow = 0.0f;

	shadow += ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowCoord, input.shadowUV.z,int2(0,0));
	shadow += ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowCoord, input.shadowUV.z,int2(0,1));
	shadow += ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowCoord, input.shadowUV.z,int2(1,0));
	shadow += ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowCoord, input.shadowUV.z,int2(1,1));



	l_res.LightOut = (diffuse * Alebdo.Sample(DefaultSampler,input.uv) + spec) * 0.85 + 0.15;
	l_res.LightOut *= shadow / 4;
	l_res.NormalOut = float4(input.normal,1.0f);
	l_res.SpecularOut = float4(input.shadowUV.xy, 0.0f, 1.0f);
	l_res.SpecularOut.a = input.scenePositionView.z;
	return l_res;

//#ifdef DEBUG_SHADER
//
//	if (screenPosition.x > 0.0 && screenPosition.x < 0.6)
//	{
//		return (diffuse + spec) * 0.75 + float4(0.25, 0.25, 0.25, 1.0);
//	}
//	else if(screenPosition.x > 0.6 && screenPosition.x < 1.0)
//	{
//		return diffuseDebug;
//
//	}
//	else
//	{
//		return sliceColor[clusterPosition.z];
//
//	}
//#endif

}