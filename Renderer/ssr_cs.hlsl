
RWTexture2D<float4> SSR : register(u0);

[numthreads(32,8,1)]
void main
(
	uint3 globalID : SV_DispatchThreadID,
	uint3 groupID : SV_GroupID,
	uint3 threadID : SV_GroupThreadID,
	uint threadIndex : SV_GroupIndex
)
{
	SSR[int2(globalID.x, globalID.y)] = float4(globalID.x/1920.0, globalID.y/1080.0,0.0,1.0f);
}