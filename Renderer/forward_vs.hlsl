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

#include "shader_common.hlsli"


PSInput main(float4 position : POSITION, float3 normal : NORMAL0,float2 tex_uv : TEXCOORD0)
{
	PSInput result;
	result.scenePositionView = mul(view, position);
	result.position = mul(project, result.scenePositionView);
	result.normal = normalize(mul(view,float4(normal,0.0))).xyz;
	//result.normal = normal;
	result.color = float4(normal,1.0f);
	return result;
}



