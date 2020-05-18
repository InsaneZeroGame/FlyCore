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


PSInput main(float4 position : POSITION, float3 normal : NORMAL0,float2 tex_uv : TEXCOORD0, int4 boneIndex : BONEINDEX,float4 boneWeight : BONEWEIGHT)
{
	PSInput result;
	float4x4 modelMatrix = 
	{ 
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	if (boneIndex.x != -1)
	{
		modelMatrix  = boneMatrix[boneIndex.x] * boneWeight.x;
		modelMatrix += boneMatrix[boneIndex.y] * boneWeight.y;
		modelMatrix += boneMatrix[boneIndex.z] * boneWeight.z;
		modelMatrix += boneMatrix[boneIndex.w] * boneWeight.w;
	}
	

	modelMatrix = mul(model, modelMatrix);
	result.scenePositionView = mul(view, mul(modelMatrix,position));
	result.position = mul(project, result.scenePositionView);
	float4 shadowCoord = mul(shadowMatrix, mul(modelMatrix,position));
	result.shadowUV = shadowCoord;
	result.normal = mul(view,mul(modelMatrix,float4(normal,0.0))).xyz;
	result.uv = tex_uv;
	return result;
}



