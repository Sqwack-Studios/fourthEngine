#include "../Registers.hlsli"

Texture2D<float3>   g_emissionBuffer : register(srv_GB_EMISSION);

float4 main(float4 psin : SV_POSITION) : SV_TARGET
{
	return float4(g_emissionBuffer.Load(uint3(uint2(psin.xy), 0)), 1.0f);
}