//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************



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

cbuffer CDataBuffer : register(b0)
{
	float4x4 MVP;
	PointLight PointLights[1024];
};


PSInput main(float4 position : POSITION, float3 normal : NORMAL0,float2 tex_uv : TEXCOORD0)
{
	PSInput result;

	result.position = mul(MVP,position);
	result.color = float4(normal,1.0f);

	return result;
}



