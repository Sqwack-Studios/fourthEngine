#include "ResolveLightning.hlsli"

float4 main(float4 inps : SV_POSITION) : TARGET_COMBINE_HDR
{
	return resolveLightning(inps.xy);
}