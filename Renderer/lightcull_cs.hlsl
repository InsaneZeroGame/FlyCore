#include "shader_common.hlsli"
#define WORK_GROUP_SIZE_X 8 
#define WORK_GROUP_SIZE_Y 8 
#define WORK_GROUP_SIZE_Z 4

//Group Count
static uint GROUP_SIZE_X = 8;
static uint GROUP_SIZE_Y = 8;
static uint GROUP_SIZE_Z = 16;

static uint2  SCREEN_DIMENSION = uint2(1920,1080);



RWStructuredBuffer<LightList> LightBuffer : register(u0);
StructuredBuffer<PointLight> PointLights:register(t1);

[numthreads(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y, WORK_GROUP_SIZE_Z)]
void main(
	uint3 globalID : SV_DispatchThreadID,
	uint3 groupID : SV_GroupID,
	uint3 threadID : SV_GroupThreadID,
	uint threadIndex : SV_GroupIndex
)
{
	float zNear = zNearFar.x;
	float zFar = zNearFar.y;
	float zRange = zFar - zNear;
	float zRangePerSlice = zRange / GROUP_SIZE_Z;

	float zSlice = zNear + zRangePerSlice * groupID.z;
	float zSliceNext = zSlice + zRangePerSlice;
	uint groupIndexInCS = groupID.z * (GROUP_SIZE_X * GROUP_SIZE_Y) + groupID.y * GROUP_SIZE_X + groupID.x;
	LightBuffer[groupIndexInCS].isActive[threadIndex] = false;
	float2 tileScale = float2(4,4);	
	float2 tileBias = tileScale - groupID.xy;

	float4 c1 = float4(project._11 * tileScale.x, 0.0, tileBias.x, 0.0);
	float4 c2 = float4(0.0, -project._22 * tileScale.y, tileBias.y, 0.0);
	float4 c4 = float4(0.0, 0.0, 1.0, 0.0);
	float4 frustumPlanes[6];

	frustumPlanes[0] = c4 - c1; // 
	frustumPlanes[1] = c1; // 
	frustumPlanes[2] = c4 - c2; // 
	frustumPlanes[3] = c2; // 
	//
	frustumPlanes[4] = float4(0.0, 0.0, 1.0, -zSlice);

	frustumPlanes[5] = float4(0.0, 0.0, -1.0, zSliceNext);
    
    // Normalize frustum planes (near/far already normalized)
    [unroll]
    for (uint i = 0; i < 4; ++i)
    {
        frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
    }

	PointLight light = PointLights[threadIndex];

	// Cull: point light sphere vs tile frustum
	bool inFrustum = true;
	if (light.isActive == 1)
	{

		[unroll] for (uint i = 0; i < 6; ++i) {
			float d = dot(frustumPlanes[i], float4(light.pos));
			inFrustum = inFrustum && (d > -light.radius / light.attenutation);
		}

		LightBuffer[groupIndexInCS].isActive[threadIndex] = inFrustum;
	}
}