struct PSInput
{
	float4 position : SV_POSITION;
	float4 scenePositionView : POSITION0;
	float4 color : COLOR0;
	float3 normal:NORMAL;

};


struct PointLight
{
	float4 pos;
	float4 color;
	float radius;
	uint isActive;
};

cbuffer CDataBuffer : register(b0)
{
	float4x4 project;
	float4x4 view;
	float4x4 projInverse;
	float4 zNearFar;
};

struct LightList
{
	uint isActive[256];
};

float linstep(float min, float max, float v)
{
	return saturate((v - min) / (max - min));
}

StructuredBuffer<LightList> LightBuffer : register(t1);
StructuredBuffer<PointLight> PointLights : register(t2);

static uint2  SCREEN_DIMENSION = uint2(1920, 1080);
static float4 sliceColor[8] = {
	float4(0.0,0.0,0.0,1.0f),
	float4(0.1,0.1,0.1,1.0f),
	float4(0.2,0.2,0.2,1.0f),
	float4(0.3,0.3,0.3,1.0f),
	float4(0.4,0.4,0.4,1.0f),
	float4(0.5,0.5,0.5,1.0f),
	float4(0.6,0.6,0.6,1.0f),
	float4(0.7,0.7,0.7,1.0f),
};


float4 main(PSInput input) : SV_TARGET
{
	float zNear = zNearFar.x;
	float zFar = zNearFar.y;
	float zRange = zFar - zNear;
	float zRangePerSlice = zRange / 8.0;

	float zPosition = (input.scenePositionView.z - zNear) / zRangePerSlice;
	uint zIndex = zPosition;
	
	float2 screenPosition = input.position.xy / SCREEN_DIMENSION.xy;
	uint3 clusterPosition = uint3(screenPosition * uint2(16,8), zIndex);

	float4 res_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	uint ClusterIndex = clusterPosition.z * (16 * 8) + clusterPosition.y * 16 + clusterPosition.x;
	float4 diffuse = float4(0.0, 0.0, 0.0, 1.0);
	float4 spec = 0.0f;
	float4 colorStep = float4(1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0, 1.0);

	float4 diffuseDebug = 0.0f;

	for (uint i = 0; i < 1; ++i)
	{
		//if (LightBuffer[ClusterIndex].isActive[i] == 1)
		//{
		float3 lightDir = PointLights[i].pos.xyz - input.scenePositionView.xyz;
		float3 lightDirNormalized = normalize(lightDir);
		//float3 viewDir = normalize(float3(0.0,0.0,0.0) - input.scenePositionView.xyz);
		//float3 halfwarDir = normalize(lightDir + viewDir);

		float lightDistSq = dot(lightDir, lightDir);
		float invLightDist = rsqrt(lightDistSq);
		lightDir *= invLightDist;

		// modify 1/d^2 * R^2 to fall off at a fixed radius
		// (R/d)^2 - d/R = [(1/d^2) - (1/R^2)*(d/R)] * R^2
		float distanceFalloff = PointLights[i].radius * PointLights[i].radius * (invLightDist * invLightDist);
		distanceFalloff = max(0, distanceFalloff - rsqrt(distanceFalloff));
		diffuse += dot(input.normal, lightDirNormalized) * float4(PointLights[i].color);
		//diffuse += PointLights[i].color * 0.02;
		//spec += pow(max(dot(input.normal, halfwarDir), 0.0),15.0) * PointLights[i].color;
		//diffuse += PointLights[i].color * 0.1;
		//diffuseDebug += colorStep;
		//}
	}
	

	//return float4(1,0.5,0.5,1.0);
	//return PointLights[0].color;
	//return float4(input.normal,1.0f);
	return diffuse;

	if (screenPosition.x > 0.0 && screenPosition.x < 0.5)
	{

	}
	else if(screenPosition.x > 0.5 && screenPosition.x < 1.0)
	{
		return sliceColor[zIndex];
	}
	else
	{
		return diffuseDebug;
	}

}