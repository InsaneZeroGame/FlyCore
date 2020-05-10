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




SSRPSinput main(float4 position : POSITION, float3 normal : NORMAL0,float2 tex_uv : TEXCOORD0, float4 bone : BONE)
{
	SSRPSinput result;
	result.position = position;

	result.cameraRay = float4(tex_uv * 2.0f - 1.0f,1.0f,1.0f);
	result.cameraRay = mul(projInverse,  result.cameraRay);
	result.cameraRay = result.cameraRay / result.cameraRay.w;
	result.uv = tex_uv;
	return result;
}



