struct PSInput
{
	float4 position : SV_POSITION;
	float4 scenePositionView : positionView;
	float2 uv : TEXCOORD0;
	float3 shadowUV : TEXCOORD1;
	float3 normal:NORMAL;

};

struct ShadowOut
{
	float4 position:SV_POSITION;
};

struct SSRPSinput
{
	float4 position : SV_POSITION;
	float4 cameraRay : TEXCOORD0;
	float2 uv:TEXCOORD1;
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
	float4x4 shadowMatrix;
	float4 zNearFar;
};

struct LightList
{
	uint isActive[256];
};