

struct PSInput
{
	float4 position : SV_POSITION;
	float4 scenePositionView : POSITION;
	float4 color : COLOR;
};


struct PointLight
{
	float4 pos;
	float4 color;
	float attenutation;
	uint isActive;
};

struct LightList
{
	uint isActive[256];
};

StructuredBuffer<LightList> LightBuffer : register(t1);

static float4 sliceColor[8] = {
	float4(1.0,0.0,0.0,1.0f),
	float4(0.0,1.0,0.0,1.0f),
	float4(0.0,0.0,1.0,1.0f),
	float4(1.0,1.0,0.0,1.0f),
	float4(1.0,0.0,1.0,1.0f),
	float4(0.0,1.0,1.0,1.0f),
	float4(0.5,0.5,0.5,1.0f),
	float4(1.0,1.0,1.0,1.0f),
};

static float zNear = 0.01;
static float zFar = 55.0;
static uint2  SCREEN_DIMENSION = uint2(1920, 1080);


float4 main(PSInput input) : SV_TARGET
{

	float zPosition = (input.scenePositionView.z - zNear) / (zFar - zNear);
	uint zIndex = zPosition * 8;
	//float z = input.position.z;
	////z = 2.0 * zNear * zFar / (zFar + zNear - z * (zFar - zNear));
	//unsigned int numSlices = 8;
	//unsigned int slice = log(z) * numSlices / log(zFar / zNear) - numSlices * log(zNear) / (log(zFar / zNear));
	//
	float2 screenPosition = input.position.xy / SCREEN_DIMENSION.xy;
	uint3 clusterPosition = uint3(screenPosition * uint2(16,8), zIndex);

	float4 colorStep = float4(16.0 / 256, 16.0 / 256, 16.0 / 256, 1.0f);
	float4 res_color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	for (uint i = 0; i < 256; ++i)
	{
		if (LightBuffer[clusterPosition.z * (16 * 8) + clusterPosition.y * 16 + clusterPosition.x].isActive[i] == 1)
		{
			res_color += colorStep;
		}
	}

	return res_color;
}