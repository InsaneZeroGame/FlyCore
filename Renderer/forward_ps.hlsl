struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};


struct PointLight
{
	float4 pos;
	float4 color;
	float attenutation;
};

RWStructuredBuffer<PointLight> LightBuffer : register(u0);

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
static float zFar = 15.0;
float4 main(PSInput input) : SV_TARGET
{
	float z = input.position.z;
    //z = 2.0 * zNear * zFar / (zFar + zNear - z * (zFar - zNear));
	unsigned int numSlices = 8;
	unsigned int slice = log(z) * numSlices / log(zFar / zNear) - numSlices * log(zNear) / (log(zFar / zNear));

	return sliceColor[slice];
}