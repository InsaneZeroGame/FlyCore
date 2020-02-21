#define WORK_GROUP_SIZE_X 8 
#define WORK_GROUP_SIZE_Y 8 
#define WORK_GROUP_SIZE_Z 4

struct PointLight
{
	float4 pos;
	float4 color;
	float attenutation;
};

cbuffer CDataBuffer : register(b0)
{
	float4x4 project;
	float4x4 view;
	PointLight PointLights[256];
};


RWStructuredBuffer<PointLight> LightBuffer : register(u0);


[numthreads(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y, WORK_GROUP_SIZE_Z)]
void main(
	uint3 globalID : SV_DispatchThreadID,
	uint3 groupID : SV_GroupID,
	uint3 threadID : SV_GroupThreadID,
	uint threadIndex : SV_GroupIndex
)
{
	LightBuffer[WORK_GROUP_SIZE_X].pos = float4(1.0f, 1.0f, 1.0f, 1.0f);
	LightBuffer[WORK_GROUP_SIZE_X].color = float4(2.0f, 2.0f, 2.0f, 2.0f);
	LightBuffer[WORK_GROUP_SIZE_X].attenutation = 3.0f;
}