#define WORK_GROUP_SIZE_X 16 
#define WORK_GROUP_SIZE_Y 8 
#define WORK_GROUP_SIZE_Z 8

struct PointLight
{
	float4 pos;
	float4 color;
	float attenutation;
};

cbuffer CDataBuffer : register(b0)
{
	float4x4 MVP;
	PointLight PointLights[1024];
};


RWStructuredBuffer<PointLight> LightBuffer : register(u0);


[numthreads(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y, WORK_GROUP_SIZE_Z)]
void main()
{
	LightBuffer[WORK_GROUP_SIZE_X].pos = float4(1.0f, 1.0f, 1.0f, 1.0f);
	LightBuffer[WORK_GROUP_SIZE_X].color = float4(2.0f, 2.0f, 2.0f, 2.0f);
	LightBuffer[WORK_GROUP_SIZE_X].attenutation = 3.0f;
}