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



cbuffer CDataBuffer : register(b0)
{
	float4x4 project;
	float4x4 view;
	float4x4 projInverse;
	float zNear;
	float zFar;
	PointLight PointLights[256];
};


PSInput main(float4 position : POSITION, float3 normal : NORMAL0,float2 tex_uv : TEXCOORD0)
{
	PSInput result;
	result.scenePositionView = mul(view, position);
	result.position = mul(project, result.scenePositionView);
	result.color = float4(normal,1.0f);

	return result;
}



