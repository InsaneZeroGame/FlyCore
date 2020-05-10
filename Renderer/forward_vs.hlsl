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


PSInput main(float4 position : POSITION, float3 normal : NORMAL0,float2 tex_uv : TEXCOORD0,float4 bone : BONE)
{
	PSInput result;
	result.scenePositionView = mul(view, mul(model,position));
	result.position = mul(project, result.scenePositionView);
	float4 shadowCoord = mul(shadowMatrix, mul(model,position));
	result.shadowUV = shadowCoord.xyz/shadowCoord.w;
	result.normal = mul(view,mul(model,float4(normal,0.0))).xyz;
	result.uv = tex_uv;
	return result;
}



