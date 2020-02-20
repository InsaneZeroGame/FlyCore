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

float4 main(PSInput input) : SV_TARGET
{

	return input.color;
}