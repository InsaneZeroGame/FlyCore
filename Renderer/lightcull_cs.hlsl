#define WORK_GROUP_SIZE_X 16 
#define WORK_GROUP_SIZE_Y 8 
#define WORK_GROUP_SIZE_Z 8

cbuffer CDataBuffer : register(b0)
{
	float4x4 MVP;
};


[numthreads(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y, WORK_GROUP_SIZE_Z)]
void main()
{
	
}