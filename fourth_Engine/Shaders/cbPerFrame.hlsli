#include "Registers.hlsli"

#ifndef _cbPER_FRAME_HLSL
#define _cbPER_FRAME_HLSL

cbuffer cb_perFrame : register(cb_PER_FRAME)
{
	float4   g_resolution;

	float4   g_mousePos;

	float    g_time;
	float    g_deltaTime;
	float    g_EV100;
	bool     g_validateLuminance;

	bool     g_enableDiffuse;
	bool     g_enableSpecular;
	bool     g_enableIBL;
	bool     g_overwriteRoughness;

	float    g_overwrittenRoughness;
	uint     g_reflectionMips;
	uint     g_MRP_Flags;
	uint     g_mainRTVSubsamples;
}

#endif //_cbPER_FRAME_HLSL
