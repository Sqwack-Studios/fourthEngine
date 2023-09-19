#include "../Registers.hlsli"
#include "../sSamplers.hlsli"
#include "IBLOperations.hlsli"

struct VSOut
{
	float4 vertexClip : SV_POSITION;
	float2 uv : UV;
};

cbuffer SamplingProperties : register(cb_IBL_GEN)
{
	uint g_numSamples;
}

float4 main(VSOut vsOut) : SV_TARGET
{

	float2 integral = computeIBLSpecularReflectance(g_numSamples, 1.0f - vsOut.uv.y, vsOut.uv.x);

	return float4(integral, 1.0f, 1.0f);
}