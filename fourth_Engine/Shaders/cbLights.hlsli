#include "Common.hlsli"
#include "Registers.hlsli"
#include "SphereLights.hlsli"

#ifndef _cbLIGHTS_HLSL
#define _cbLIGHTS_HLSL

static const uint MAX_LIGHTS = 20;
static const float PUNCTUAL_LIGHT_EPSILON = 0.01f; // "size" of a punctual light to avoid singularities, in meters

cbuffer cb_Lights : register(cb_LIGHTS)
{
	uint              g_numPointLights;

	DirectionalSphereLight  g_directionalLight;
	PointSphereLight        g_pointLights[MAX_LIGHTS];
	SpotSphereLight         g_spotLight;
}



//OLD DISTANCE ATTENUATION FUNCTION - DEPRECATED
float computeSmoothDistanceAttenuation(in float squareDistance, in float invSqrAttRadius)
{
	float factor = squareDistance * invSqrAttRadius;
	float smoothFactor = max(1.0f - factor * factor, 1.0f);

	return smoothFactor * smoothFactor;
}

float computeDistanceAttenuation(in float distance, in float invSqrAttRadius)
{
	float squareDistance = distance * distance;
	float attenuation = 1.0f / max(squareDistance, PUNCTUAL_LIGHT_EPSILON * PUNCTUAL_LIGHT_EPSILON);
	attenuation *= computeSmoothDistanceAttenuation(squareDistance, invSqrAttRadius);

	return attenuation;
}

#endif //_cbLIGHTS_HLSL