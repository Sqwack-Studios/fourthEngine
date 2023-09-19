#include "../RenderTargets.hlsli"

struct PSIn
{
	float4 posClip: SV_POSITION;
	float3 posWorld : WPOSITION;
	float3 normalWorld: WNORMAL;
};


float4 main(PSIn input) : TARGET_EMISSION
{
	float4 outColor = float4(0.5f, 0.5f, 0.5f, 0.5f) + float4(0.5f, 0.5f, 0.5f, 0.5f) * float4(input.normalWorld.xyz, 1.0f);
	
	return outColor;
}