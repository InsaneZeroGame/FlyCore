#include "shader_common.hlsli"

#define PCF_SAMPLE_COUNT 64
#define BLOCKER_SEARCH_COUNT 64
#define LIGHT_SIZE 1.50

float linstep(float min, float max, float v)
{
	return saturate((v - min) / (max - min));
}

StructuredBuffer<LightList> LightBuffer : register(t1);
StructuredBuffer<PointLight> PointLights : register(t2);
Texture2D <float4> Alebdo : register(t3);
Texture2D <float> ShadowMap : register(t4);
SamplerState DefaultSampler :register(s0);
SamplerComparisonState ShadowSampler :register(s1);

static float4 sliceColor[16] = {
	float4(0.0,0.0,0.0,1.0f),
	float4(0.1,0.1,0.1,1.0f),
	float4(0.2,0.2,0.2,1.0f),
	float4(0.3,0.3,0.3,1.0f),
	float4(0.4,0.4,0.4,1.0f),
	float4(0.5,0.5,0.5,1.0f),
	float4(0.6,0.6,0.6,1.0f),
	float4(0.7,0.7,0.7,1.0f),
	float4(0.8,0.8,0.8,1.0f),
	float4(0.3,0.3,0.3,1.0f),
	float4(0.4,0.4,0.4,1.0f),
	float4(0.5,0.5,0.5,1.0f),
	float4(0.6,0.6,0.6,1.0f),
	float4(0.7,0.7,0.7,1.0f),
	float4(0.8,0.8,0.8,1.0f),
	float4(0.0,0.0,0.0,1.0f),

};

static float2 poissonDisk[16] = {
 float2(-0.94201624, -0.39906216),
 float2(0.94558609, -0.76890725),
 float2(-0.094184101, -0.92938870),
 float2(0.34495938, 0.29387760),
 float2(-0.91588581, 0.45771432),
 float2(-0.81544232, -0.87912464),
 float2(-0.38277543, 0.27676845),
 float2(0.97484398, 0.75648379),
 float2(0.44323325, -0.97511554),
 float2(0.53742981, -0.47373420),
 float2(-0.26496911, -0.41893023),
 float2(0.79197514, 0.19090188),
 float2(-0.24188840, 0.99706507),
 float2(-0.81409955, 0.91437590),
 float2(0.19984126, 0.78641367),
 float2(0.14383161, -0.14100790)
};


static const float2 Poisson64[64] = {
	float2(-0.934812, 0.366741),
	float2(-0.918943, -0.0941496),
	float2(-0.873226, 0.62389),
	float2(-0.8352, 0.937803),
	float2(-0.822138, -0.281655),
	float2(-0.812983, 0.10416),
	float2(-0.786126, -0.767632),
	float2(-0.739494, -0.535813),
	float2(-0.681692, 0.284707),
	float2(-0.61742, -0.234535),
	float2(-0.601184, 0.562426),
	float2(-0.607105, 0.847591),
	float2(-0.581835, -0.00485244),
	float2(-0.554247, -0.771111),
	float2(-0.483383, -0.976928),
	float2(-0.476669, -0.395672),
	float2(-0.439802, 0.362407),
	float2(-0.409772, -0.175695),
	float2(-0.367534, 0.102451),
	float2(-0.35313, 0.58153),
	float2(-0.341594, -0.737541),
	float2(-0.275979, 0.981567),
	float2(-0.230811, 0.305094),
	float2(-0.221656, 0.751152),
	float2(-0.214393, -0.0592364),
	float2(-0.204932, -0.483566),
	float2(-0.183569, -0.266274),
	float2(-0.123936, -0.754448),
	float2(-0.0859096, 0.118625),
	float2(-0.0610675, 0.460555),
	float2(-0.0234687, -0.962523),
	float2(-0.00485244, -0.373394),
	float2(0.0213324, 0.760247),
	float2(0.0359813, -0.0834071),
	float2(0.0877407, -0.730766),
	float2(0.14597, 0.281045),
	float2(0.18186, -0.529649),
	float2(0.188208, -0.289529),
	float2(0.212928, 0.063509),
	float2(0.23661, 0.566027),
	float2(0.266579, 0.867061),
	float2(0.320597, -0.883358),
	float2(0.353557, 0.322733),
	float2(0.404157, -0.651479),
	float2(0.410443, -0.413068),
	float2(0.413556, 0.123325),
	float2(0.46556, -0.176183),
	float2(0.49266, 0.55388),
	float2(0.506333, 0.876888),
	float2(0.535875, -0.885556),
	float2(0.615894, 0.0703452),
	float2(0.637135, -0.637623),
	float2(0.677236, -0.174291),
	float2(0.67626, 0.7116),
	float2(0.686331, -0.389935),
	float2(0.691031, 0.330729),
	float2(0.715629, 0.999939),
	float2(0.8493, -0.0485549),
	float2(0.863582, -0.85229),
	float2(0.890622, 0.850581),
	float2(0.898068, 0.633778),
	float2(0.92053, -0.355693),
	float2(0.933348, -0.62981),
	float2(0.95294, 0.156896)
};
struct MRT
{
	float4 LightOut : COLOR0;
	float4 NormalOut : COLOR1;
	float4 SpecularOut:COLOR2;
};

void FindBlocker(out float averageDepth, out float numOfBlockers, float3 shadowCoord,float zEye)
{
	
	float searchWidth = LIGHT_SIZE * (zEye - zNearFar[0]) / zEye;

	 
	float blockerSum = 0;
	numOfBlockers = 0;

	for (int i = 0; i < BLOCKER_SEARCH_COUNT; ++i)
	{
		float shadowMapDepth = ShadowMap.Sample(
			DefaultSampler,
			shadowCoord.xy + Poisson64[i] * searchWidth );
			if (shadowMapDepth < shadowCoord.z)
			{
				numOfBlockers++;
				blockerSum += shadowMapDepth;
			}
	}
	averageDepth = blockerSum / numOfBlockers;
}

float PCF_Filter(float2 uv, float zEye, float filterRadiusUV)
{
	float sum = 0.0f;
	for (int i = 0; i < PCF_SAMPLE_COUNT; ++i)
	{
		float2 offset = Poisson64[i] * filterRadiusUV;
		sum += ShadowMap.SampleCmpLevelZero(ShadowSampler, uv + offset / float2(800.0f, 600.0f), zEye);
	}
	return sum / PCF_SAMPLE_COUNT;
}

float PCSS(float3 shadowCoord,float zEye)
{
	
	//	1.Find Blocker
	//Find Blocker
	float numOfBlocker = 0.0f;
	float averageDepth = 0.0f;
	FindBlocker(averageDepth, numOfBlocker, shadowCoord, zEye);
	//return numOfBlocker / BLOCKER_SEARCH_COUNT;
	//	2.
	if (numOfBlocker == 0.0f)
	{
		return 1.0f;			//Early Exit
	}
	//return 0.0f;

	float penumbraRatio = averageDepth * (zEye - averageDepth);
	float filterRadiusUV = penumbraRatio * LIGHT_SIZE * zNearFar[0] / zEye;
	//return filterRadiusUV;
	//PCF Filter
	return PCF_Filter(shadowCoord.xy, shadowCoord.z - 0.005, filterRadiusUV);

}


float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


MRT main(PSInput input) : SV_TARGET
{
	float zNear = zNearFar.x;
	float zFar = zNearFar.y;
	float zPosition = (input.scenePositionView.z - zNear) / (zFar - zNear);
	//float zPosition = log(input.scenePositionView.z)
	float2 screenPosition = input.position.xy / SCREEN_DIMENSION.xy;
	uint3 clusterPosition = uint3(screenPosition * uint2(GROUP_SIZE_X, GROUP_SIZE_Y), zPosition * GROUP_SIZE_Z);


	//Note : This triggers a memory violation,Clmap it for now,later figures the correct math.
	uint ClusterIndex = clusterPosition.z * (GROUP_SIZE_X * GROUP_SIZE_Y) + clusterPosition.y * GROUP_SIZE_X + clusterPosition.x;
	ClusterIndex = clamp(ClusterIndex, 0, 255);
	float4 diffuse = float4(0.0, 0.0, 0.0, 1.0);
	float4 spec = 0.0f;
	float4 colorStep = 1.0/256.0;

	float4 diffuseDebug = 0.0f;
	MRT l_res;
	float3 SceneViewPos = input.scenePositionView.xyz / input.scenePositionView.w;

	float3 N = normalize(input.normal);
	float3 V = normalize(float3(0.0, 0.0, 0.0) - SceneViewPos);
	float3 Lo = 0.0;
	float3 F0 = float3(0.56, 0.57, 0.58);

	float roughness = screenPosition.x;
	float metallic = 1.0- roughness;
	//float3 albedo = Alebdo.Sample(DefaultSampler, input.uv).rgb;
	float3 albedo = 0.75;
	for (uint i = 0; i < 256; ++i)
	{
		if (LightBuffer[ClusterIndex].isActive[i] == 1)
		{
			float3 LightViewPos = mul(view, PointLights[i].pos).xyz;
			float3 L = normalize(LightViewPos - SceneViewPos);
			float3 H = normalize(L + V);
			float distance = length(LightViewPos - SceneViewPos);
			float attenuation = 1.0 / (distance * distance);
			float3 radiance = PointLights[i].color.rgb * attenuation;

			// cook-torrance brdf
			float NDF = DistributionGGX(N, H, roughness);
			float G = GeometrySmith(N, V, L, roughness);
			float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

			float3 kS = F;
			float3 kD = 1.0 - kS;
			kD *= 1.0 - metallic;

			float3 numerator = NDF * G * F;
			float denominator = 4.0 * dot(N, V) * dot(N, L);
			float3 specular = numerator / max(denominator, 0.001);

			// add to outgoing radiance Lo
			float NdotL = max(dot(N, L), 0.0);
			Lo += (kD * albedo / PI + specular) * radiance * NdotL;

			diffuseDebug += colorStep;
		}
	}
	float2 shadowCoord;
	shadowCoord.x = input.shadowUV.x * 0.5 + 0.5;
	shadowCoord.y = -input.shadowUV.y * 0.5 + 0.5;
	float shadow = PCSS(float3(shadowCoord, input.shadowUV.z),input.scenePositionView.z);

	l_res.LightOut = float4(Lo,1.0) * shadow * materialColor;
	//l_res.LightOut = diffuseDebug;
	float gamma = 2.2;
	//l_res.LightOut = l_res.LightOut / (l_res.LightOut + 1.0f);
	float exposure = 5.0f;
	l_res.LightOut = 1.0 - exp(-l_res.LightOut * exposure);
	l_res.LightOut = pow(abs(l_res.LightOut), 1.0 / gamma);
	//l_res.LightOut *= Alebdo.Sample(DefaultSampler, input.uv);
	l_res.NormalOut = float4(input.normal,1.0f);
	l_res.SpecularOut = float4(input.shadowUV.xy, 0.0f, 1.0f);
	l_res.SpecularOut.a = input.scenePositionView.z;

	//l_res.LightOut = float4(shadow, 0.0, 0.0, 1.0);
	return l_res;

//#ifdef DEBUG_SHADER
//
//	if (screenPosition.x > 0.0 && screenPosition.x < 0.6)
//	{
//		return (diffuse + spec) * 0.75 + float4(0.25, 0.25, 0.25, 1.0);
//	}
//	else if(screenPosition.x > 0.6 && screenPosition.x < 1.0)
//	{
//		return diffuseDebug;
//
//	}
//	else
//	{
//		return sliceColor[clusterPosition.z];
//
//	}
//#endif

}