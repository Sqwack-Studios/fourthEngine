#include "../sSamplers.hlsli"

#ifndef _DISSOLUTION_SHADER_HLSLI
#define _DISSOLUTION_SHADER_HLSLI
////DISSOLUTION EFFECT
//Texture2D<float1> g_dissolutionNoise : register(srv_DIS_NOISE);

float sampleNoise(in float2 uv, in float timeScale, in float currentTime)
{
	float noise = g_dissolutionNoise.Sample(g_pointWrap, uv * 3.0f);

	float scaledNoise = timeScale * noise;

	return scaledNoise;
}

#endif //_DISSOLUTION_SHADER_HLSLI