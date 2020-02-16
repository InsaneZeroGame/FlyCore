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

float4x4 MVP : register(b0);

PSInput main(float4 position : POSITION, float4 color : COLOR)
{
	PSInput result;

	result.position = mul(position,MVP);
	result.color = color;

	return result;
}



