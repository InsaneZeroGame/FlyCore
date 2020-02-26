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
	float4 zNearFar;
};


RWStructuredBuffer<LightList> LightBuffer : register(u0);
StructuredBuffer<PointLight> PointLights:register(t1);

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
	float zNear = zNearFar.x;
	float zFar = zNearFar.y;
	float zRange = zFar - zNear;
	float zRangePerSlice = zRange / 8.0;

	float zSlice = zNear + zRangePerSlice * groupID.z;
	float zSliceNext = zSlice + zRangePerSlice;
	uint groupIndexInCS = groupID.z * (GROUP_SIZE_X * GROUP_SIZE_Y) + groupID.y * GROUP_SIZE_X + groupID.x;


	float2 tileNum = float2(16,8);
    float2 tileCenterOffset = float2(groupID.xy) * 2 + float2(1.0f, 1.0f) - tileNum;

    // Now work out composite projection matrix
    // Relevant matrix columns for this tile frusta
    float4 c1 = float4(-project._11 * tileNum.x, 0.0f, tileCenterOffset.x, 0.0f);
    float4 c2 = float4(0.0f, -project._22 * tileNum.y, -tileCenterOffset.y, 0.0f);
    float4 c4 = float4(0.0f, 0.0f, 1.0f, 0.0f);

    // Derive frustum planes
    float4 frustumPlanes[6];
    // Sides
    frustumPlanes[0] = c4 - c1;
    frustumPlanes[1] = c4 + c1;
    frustumPlanes[2] = c4 - c2;
    frustumPlanes[3] = c4 + c2;

    // Near/far
    frustumPlanes[4] = float4(0.0f, 0.0f,  1.0f, -zSlice);
    frustumPlanes[5] = float4(0.0f, 0.0f, -1.0f,  zSliceNext);
    
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
			inFrustum = inFrustum && (d >= -light.radius);
		}

		LightBuffer[groupIndexInCS].isActive[threadIndex] = inFrustum;
	}
}