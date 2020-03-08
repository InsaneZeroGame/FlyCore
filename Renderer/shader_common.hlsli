struct PSInput
{
	float4 position : SV_POSITION;
	float4 scenePositionView : positionView;
	float4 color : COLOR0;
	float3 normal:NORMAL;

};


struct PointLight
{
	float4 pos;
	float4 color;
	float radius;
	float attenutation;
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