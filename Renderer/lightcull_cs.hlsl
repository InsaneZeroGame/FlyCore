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
struct Plane
{
	float3 N;   // Plane normal.
	float  d;   // Distance to origin.
};


Plane ComputePlane(float3 p0, float3 p1, float3 p2)
{
	float3 v0 = p1 - p0;
	float3 v2 = p2 - p0;
	float3 normal = normalize(cross(v0, v2));
	// Compute the distance to the origin using p0.
	float d = dot(normal, p0);
	Plane plane;
	plane.N = normal;
	plane.d = d;
	return plane;
}


struct Sphere
{
	float3 c;   // Center point.
	float  r;   // Radius.
};


struct Frustum
{
	Plane planes[4];   // left, right, top, bottom frustum planes.
};

bool SphereInsidePlane(Sphere sphere, Plane plane)
{
	return dot(plane.N, sphere.c) - plane.d < -sphere.r;
}

bool SphereInsideFrustum(Sphere sphere, Frustum frustum, float l_zNear, float l_zFar)
{
	bool result = true;

	// First check depth
	// Note: Here, the view vector points in the -Z axis so the 
	// far depth value will be approaching -infinity.
	if (sphere.c.z - sphere.r > l_zFar || sphere.c.z + sphere.r < l_zNear)
	{
		result = false;
	}

	// Then check frustum planes
	for (int i = 0; i < 4 && result; i++)
	{
		if (SphereInsidePlane(sphere, frustum.planes[i]))
		{
			result = false;
		}
	}

	return result;
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

	uint groupIndexInCS = groupID.z * (GROUP_SIZE_X * GROUP_SIZE_Y) + groupID.y * GROUP_SIZE_X + groupID.x;

	float zNear = zNearFar.x;
	float zFar = zNearFar.y;
	float zRange = zFar - zNear;
	float zRangePerSlice = zRange / 8.0;

	float zSlice = zNear + zRangePerSlice * groupID.z;
	float zSliceNext = zSlice + zRangePerSlice;


	float4 viewSpacePlane[4];
	float4 frustumPlanes[6];

	float tileSizeX = 1.0 / 16.0f;
	float tileSizeY = 1.0 / 8.0f;

	viewSpacePlane[0] = float4(groupID.x * tileSizeX, groupID.y * tileSizeY, 1.0f, 1.0f);
	viewSpacePlane[1] = float4((groupID.x + 1) * tileSizeX, groupID.y * tileSizeY, 1.0f, 1.0f);
	viewSpacePlane[2] = float4(groupID.x * tileSizeX, (groupID.y + 1) * tileSizeY, 1.0f, 1.0f);
	viewSpacePlane[3] = float4((groupID.x + 1) * tileSizeX, (groupID.y + 1) * tileSizeY, 1.0f, 1.0f);

	for (int i = 0; i < 4; i++)
	{
		viewSpacePlane[i] = ScreenToView(viewSpacePlane[i]);
	}
	float3 eyePos = float3(0.0f, 0.0f, 0.0f);

	Frustum frustum;

	frustum.planes[0] = ComputePlane(eyePos, viewSpacePlane[2].xyz, viewSpacePlane[0].xyz);
	frustum.planes[1] = ComputePlane(eyePos, viewSpacePlane[1].xyz, viewSpacePlane[3].xyz);
	frustum.planes[2] = ComputePlane(eyePos, viewSpacePlane[0].xyz, viewSpacePlane[1].xyz);
	frustum.planes[3] = ComputePlane(eyePos, viewSpacePlane[3].xyz, viewSpacePlane[2].xyz);



	PointLight light = PointLights[threadIndex];

	// Cull: point light sphere vs tile frustum
	bool inFrustum = true;
	if (light.isActive == 1)
	{
		Sphere l_sephere;
		l_sephere.c = light.pos.xyz;
		l_sephere.r = light.radius;

		bool inFrustum = SphereInsideFrustum(l_sephere, frustum,zSlice,zSliceNext);



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