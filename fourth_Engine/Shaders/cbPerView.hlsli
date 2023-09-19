#include "Registers.hlsli"

#ifndef _cbPER_VIEW_HLSL
#define _cbPER_VIEW_HLSL


cbuffer cbPerView : register(cb_PER_VIEW)
{
	//VP matrices
	row_major float4x4 g_VP;
	row_major float4x4 g_invVP;

	//projection
	row_major float4x4 g_P;
	//invView    cameraToWorld
	row_major float4x4 g_invV;
	//invProj
	row_major float4x4 g_invP;
	//View worldToCamera
	row_major float4x4 g_V;

	//Frustum
	float4    g_frustumCornersDirections[3]; //Directions from camera to corners. Corners were unprojected from clip space, but not translated.
	float     g_nearPlane;
	float     g_farPlane;
	float     g_viewportWidth;
	float     g_viewportHeight;
	float     g_viewportAR;
}

#endif //_cbPER_VIEW_HLSL
