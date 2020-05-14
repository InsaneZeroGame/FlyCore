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

cbuffer PushConstants : register(b5)
{
	float4x4 model;
	float4 materialColor;
}

cbuffer AnimBuffer : register(b6)
{
	float4x4 boneMatrix[60];
}

struct LightList
{
	uint isActive[256];
};

struct SkyboxPSInput {
	float4 position: SV_POSITION;
	float4 uv : POSITION;

};

//Group Count
static uint GROUP_SIZE_X = 8;
static uint GROUP_SIZE_Y = 8;
static uint GROUP_SIZE_Z = 16;
static uint2  SCREEN_DIMENSION = uint2(1920, 1080);
static float PI = 3.14159265354;