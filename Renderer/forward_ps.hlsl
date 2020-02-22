struct PSInput
{
	float4 position : SV_POSITION;
	float4 scenePositionView : POSITION;
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
	float zNear;
	float zFar;
	PointLight PointLights[256];
};

struct LightList
{
	uint isActive[256];
};

StructuredBuffer<LightList> LightBuffer : register(t1);
static uint2  SCREEN_DIMENSION = uint2(1920, 1080);


float4 main(PSInput input) : SV_TARGET
{


	uint zIndex = log(input.scenePositionView.z) * 8 / log(zFar/zNear) - 8 * log(zNear)/log(zFar/zNear);
	
	float2 screenPosition = input.position.xy / SCREEN_DIMENSION.xy;
	uint3 clusterPosition = uint3(screenPosition * uint2(16,8), zIndex);

	float4 res_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
	uint ClusterIndex = clusterPosition.z * (16 * 8) + clusterPosition.y * 16 + clusterPosition.x;
	float4 diffuse = 0.0f;
	float4 spec = 0.0f;
	for (uint i = 0; i < 256; ++i)
	{
		if (LightBuffer[ClusterIndex].isActive[i] == 1)
		{
			float distance = length(PointLights[i].pos.xyz - input.scenePositionView.xyz);
			float3 lightDir = normalize(PointLights[i].pos.xyz - input.scenePositionView.xyz);
			float3 viewDir = normalize(float3(0.0,0.0,0.0) - input.scenePositionView.xyz);
			float3 halfwarDir = normalize(lightDir + viewDir);
			float attenuation = pow(clamp(1 - pow((distance / PointLights[i].radius), 4.0), 0.0, 1.0), 2.0) / (1.0 + (distance * distance));
			diffuse += dot(lightDir,viewDir) * PointLights[i].color * attenuation;
			spec += pow(max(dot(input.normal, halfwarDir), 0.0),15.0) * PointLights[i].color;
		}
	}

	return diffuse;
}