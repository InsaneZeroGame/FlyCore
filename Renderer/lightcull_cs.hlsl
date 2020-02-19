#define WORK_GROUP_SIZE_X 16 
#define WORK_GROUP_SIZE_Y 8 
#define WORK_GROUP_SIZE_Z 8

cbuffer CDataBuffer : register(b0)
{
	float4x4 MVP;
};

struct PointLight
{
	float4 pos;
	float4 color;
	float attenutation;
};

RWStructuredBuffer<PointLight> LightBuffer : register(u0);


[numthreads(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y, WORK_GROUP_SIZE_Z)]
void main()
{
	LightBuffer[WORK_GROUP_SIZE_X].pos = float4(1.0f, 1.0f, 1.0f, 1.0f);
	LightBuffer[WORK_GROUP_SIZE_X].color = float4(2.0f, 2.0f, 2.0f, 2.0f);
	LightBuffer[WORK_GROUP_SIZE_X].attenutation = 3.0f;
}