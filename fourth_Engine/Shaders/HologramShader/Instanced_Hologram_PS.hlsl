#include "HologramEffect.hlsli"
#include "../RenderTargets.hlsli"
struct PSIn
{
	float4 posClip : SV_POSITION;
	nointerpolation float3 color : COLOR;
	float3 posLocal : LPOSITION;
	float3 posWorld : WPOSITION;
	float3 normalWorld: WNORMAL;
};


float4 main(PSIn input) : TARGET_EMISSION
{
	float3 color = colorDistortion(input.posWorld, input.normalWorld, input.posLocal, input.color).xyz;
	return float4(color, 1.0f);
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
}