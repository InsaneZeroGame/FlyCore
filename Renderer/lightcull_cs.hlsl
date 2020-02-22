#define WORK_GROUP_SIZE_X 8 
#define WORK_GROUP_SIZE_Y 8 
#define WORK_GROUP_SIZE_Z 4

//Group Count
static uint GROUP_SIZE_X = 16;
static uint GROUP_SIZE_Y = 8;
static uint GROUP_SIZE_Z = 8;

static uint2  SCREEN_DIMENSION = uint2(1920,1080);


struct PointLight
{
	float4 pos;//position in world space
	float4 color;
	float radius;
	uint isActive;
};

struct LightList
{
	uint isActive[256];
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


RWStructuredBuffer<LightList> LightBuffer : register(u0);
//groupshared bool GroupLightList[256];


// Convert clip space coordinates to view space
float4 ClipToView(float4 clip)
{
	// View space position.
	float4 view = mul(projInverse, clip);
	// Perspecitive projection.
	view = view / view.w;

	return view;
}

// Convert screen space coordinates to view space.
float4 ScreenToView(float4 screen)
{
	// Convert to normalized texture coordinates in the range [0 .. 1].
	// Convert to clip space
	float4 clip = float4(float2(screen.x, screen.y) * 2.0f - 1.0f, screen.z, screen.w);

	return ClipToView(clip);
}

float4 ComputePlane(float3 p0, float3 p1, float3 p2)
{
	float3 v0 = p1 - p0;
	float3 v2 = p2 - p0;
	float3 normal = normalize(cross(v0, v2));
	// Compute the distance to the origin using p0.
	float d = dot(normal, p0);
	return float4(normal, d);
}



[numthreads(WORK_GROUP_SIZE_X, WORK_GROUP_SIZE_Y, WORK_GROUP_SIZE_Z)]
void main(
	uint3 globalID : SV_DispatchThreadID,
	uint3 groupID : SV_GroupID,
	uint3 threadID : SV_GroupThreadID,
	uint threadIndex : SV_GroupIndex
)
{
	//GroupLightList[threadIndex] = false;
	float zRange = zFar - zNear;
	float zRangePerSlice = zRange / 8.0;

	float zSlice = zNear + zRangePerSlice * groupID.z;
	float zSliceNext = zSlice + zRangePerSlice;


	float4 viewSpacePlane[4];
	float4 frustumPlanes[6];

	float tileSizeX = 1.0f / 16.0f;
	float tileSizeY = 1.0f / 8.0f;

	viewSpacePlane[0] = float4(groupID.x * tileSizeX, groupID.y * tileSizeY, 1.0f, 1.0f);
	viewSpacePlane[1] = float4((groupID.x + 1) * tileSizeX, groupID.y * tileSizeY, 1.0f, 1.0f);
	viewSpacePlane[2] = float4(groupID.x * tileSizeX, (groupID.y + 1) * tileSizeY, 1.0f, 1.0f);
	viewSpacePlane[3] = float4((groupID.x + 1) * tileSizeX, (groupID.y + 1) * tileSizeY, 1.0f, 1.0f);

	for (int i = 0; i < 4; i++)
	{
		viewSpacePlane[i] = ScreenToView(viewSpacePlane[i]);
	}
	float3 eyePos = float3(0.0f, 0.0f, 0.0f);

	frustumPlanes[0] = ComputePlane(eyePos, viewSpacePlane[2].xyz, viewSpacePlane[0].xyz);
	frustumPlanes[1] = ComputePlane(eyePos, viewSpacePlane[1].xyz, viewSpacePlane[3].xyz);
	frustumPlanes[2] = ComputePlane(eyePos, viewSpacePlane[0].xyz, viewSpacePlane[1].xyz);
	frustumPlanes[3] = ComputePlane(eyePos, viewSpacePlane[3].xyz, viewSpacePlane[2].xyz);
	frustumPlanes[4] = float4(0.0f, 0.0f, 1.0f, -zSlice);
	frustumPlanes[5] = float4(0.0f, 0.0f, -1.0f, zSliceNext);

	// Normalize frustum planes (near/far already normalized)
	[unroll] for (uint i = 0; i < 4; ++i) {
		frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
	}

	PointLight light = PointLights[threadIndex];

	// Cull: point light sphere vs tile frustum
	bool inFrustum = true;
	if (light.isActive == 1)
	{
		[unroll] for (uint i = 0; i < 4; ++i) {
			float d = dot(frustumPlanes[i], float4(light.pos.xyz, 1.0f));
			inFrustum = inFrustum && (d >= -light.radius);
		}
		uint groupIndexInCS = groupID.z * (GROUP_SIZE_X * GROUP_SIZE_Y) + groupID.y * GROUP_SIZE_X + groupID.x;

		[branch] if (inFrustum) {
			// Append light to list
			// Compaction might be better if we expect a lot of lights
			LightBuffer[groupIndexInCS].isActive[threadIndex] = 1;
		}
		else
		{
			LightBuffer[groupIndexInCS].isActive[threadIndex] = 0;
		}


	}
}