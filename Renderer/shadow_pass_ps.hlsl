#include "shader_common.hlsli"



float4 main(ShadowOut input) :SV_TARGET
{
	return input.position;
}