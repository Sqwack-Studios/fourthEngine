#include "../cbLights.hlsli"
#include "../sSamplers.hlsli"
#ifndef _cbSHADOW_MAP_HLSL
#define _cbSHADOW_MAP_HLSL

//HELP^STRUCTS
struct CubeShadowMap
{
	row_major float4x4 faceVP[6];
};

struct SMFrustumPlanes
{
	float near;
	float far;
	float nearTexelSizeWS;
};
//UNIFORMS

//Regular color buffer: contains single VP matrices in case of directional and spotlight(textured). Contains an array of struct that represents a matrix/face in omnidirectional shadowmapping.
cbuffer ShadowMapVP : register(cb_SHADOW_MAPS)
{
	row_major float4x4 g_directionalVP;
	row_major float4x4 g_spotVP;
	CubeShadowMap      g_pointShadowMaps[MAX_LIGHTS];
	SMFrustumPlanes    g_smFrustumPlanes[3];
	//size is 1 for directional, 1 for spot, 1 for point. If I add support for multiple spotlights(texturless), then there should be a increased maximum size for this.
};

//Buffer used during  depth only pass omnidirectional shadowmapping, which uses an array of matrices and an index representing initial target slice render target
cbuffer ShadowCubePassTransforms : register(cb_SHADOW_PASS_CUBE)
{
	row_major float4x4 g_faceVP[6];
	uint               g_startIndexTarget;
}

//Computes light depth for a position, calculating (V reversed) uv coords
float computeLightDepth(in float3 pos, row_major float4x4 VP, inout float2 UV)
{
	float4 lightProj = mul(float4(pos, 1.0f), VP);
	lightProj /= lightProj.w;

	UV = lightProj.xy * 0.5f + 0.5f;
	UV.y = 1.0f - UV.y;

	return lightProj.z;
}

//Computes texel world size at certain linear depth based on near plane texel size.
//WS texel size is linearDepth * tan(fovY / 2) * 2 / resolution. Because tan and resolution is constant: tsWS(d) = d * tsWS(1.0) / near
float computeTexelWorldSize(in float linearDepth, in float nearPlane, in float nearPlaneTexelSize)
{
	return (nearPlaneTexelSize / nearPlane) * linearDepth;
}
float3 computeShadowNormalOffset(in float3 N, in float NoL, in float3 L, in float texelSizeWS)
{
	return texelSizeWS * 0.5f * 1.41421356237f * (N - 0.9f * L * NoL);
	//return float3(0.0f, 0.0f, 0.0f);
}

float smoothPCF_Directional(in Texture2D<float1> shadowMap, in float2 uv, in float currentDepth)
{
	float smoothFactor = 0.0f;
	uint width, height, levels;
	shadowMap.GetDimensions(0, width, height, levels);
	float texelSize = 1.0f / float(width);

	smoothFactor += shadowMap.SampleCmp(g_bilinearCmpClamp, uv + float2( 0.5f,  0.0f) * texelSize, currentDepth);
	smoothFactor += shadowMap.SampleCmp(g_bilinearCmpClamp, uv + float2(-0.5f,  0.0f) * texelSize, currentDepth);
	smoothFactor += shadowMap.SampleCmp(g_bilinearCmpClamp, uv + float2( 0.0f, -0.5f) * texelSize, currentDepth);
	smoothFactor += shadowMap.SampleCmp(g_bilinearCmpClamp, uv + float2( 0.0f,  0.5f) * texelSize, currentDepth);

	smoothFactor *= 0.25f;

	return smoothFactor;
}
//returns 0 in shadow, 1 not shadow. L has to be from light to target position
float computeVisibilityDirectional(in float3 pos, row_major float4x4 VP, in float3 macroN, in float3 L, in uint frustumIndex, in Texture2D<float1> shadowMap)
{
	float2 sm_UV;
	float macroNoL = dot(macroN, L);

	float currentDepth = computeLightDepth(pos, VP, sm_UV);
	//early return if depth is out of frustum
	if (currentDepth < 0.0f)
		return 1.0f;
	
	//texel size is constant in case of orthogonal projections!
	SMFrustumPlanes currentFrustumPlane = g_smFrustumPlanes[frustumIndex];


	float3 receiver = pos + computeShadowNormalOffset(macroN, macroNoL, L, currentFrustumPlane.nearTexelSizeWS);
	currentDepth = computeLightDepth(receiver, VP, sm_UV);


	float smoothVisibility = smoothPCF_Directional(shadowMap, sm_UV, currentDepth);

	return smoothVisibility;
}

float computeVisibilityPerspective(in float3 pos, row_major float4x4 VP, in float3 macroN, in float3 L, in uint frustumIndex, in Texture2D<float1> shadowMap)
{
	//To compute normal offset, we need to compute texelSize at certain depth.
	float2 sm_UV;
	float macroNoL = dot(macroN, L);

	//Sample depth at input position. Compute texel size after linearizing depth, add normal offset and sample
	float currentDepth = computeLightDepth(pos, VP, sm_UV);

	if (currentDepth < 0.0f)
	{
		return 1.0f;
	}

	//Turn off normal offset when NoL < 0.0f to avoid light leaks
	if (macroNoL > 0.0f)
	{
		SMFrustumPlanes currentFrustumPlane = g_smFrustumPlanes[frustumIndex];
		float linearDepth = linearizeDepth(currentDepth, currentFrustumPlane.near, currentFrustumPlane.far);
		float texelSize = computeTexelWorldSize(linearDepth, currentFrustumPlane.near, currentFrustumPlane.nearTexelSizeWS);

		float3 offset = computeShadowNormalOffset(macroN, macroNoL, L, texelSize);
		float3 receiver = pos + offset;
		currentDepth = computeLightDepth(receiver, VP, sm_UV);
	}

	//Add PCF!
	float visibility = shadowMap.SampleCmp(g_bilinearCmpClamp, sm_UV, currentDepth);
	return visibility;
	//float visibility = shadowMap.Sample(g_pointClamp, sm_UV);
	//return currentDepth > visibility;
}
float computeVisibilityPerspective(in float3 pos, in float3 macroN, in float3 L, in uint frustumIndex, float arrayIndex, in TextureCubeArray<float1> shadowArray)
{
	float2 sm_UV;
	float macroNoL = dot(macroN, L);

	float3 lookupDir = -L;
	//L points toward the light, and we need it to flip it to compute lookup face and UV
	float2 cubemapUV;
	float cubemapFace = lookupCubemapFrom_Direction(lookupDir, cubemapUV);

	row_major float4x4 VP = g_pointShadowMaps[arrayIndex].faceVP[cubemapFace];

	float currentDepth = computeLightDepth(pos, VP, sm_UV);
	
	//early return if depth is out of frustum
	if (currentDepth < 0.0f)
	{
		return 1.0f;
	}

	if (macroNoL > 0.0f)
	{
		SMFrustumPlanes currentFrustumPlane = g_smFrustumPlanes[frustumIndex];
		float linearDepth = linearizeDepth(currentDepth, currentFrustumPlane.near, currentFrustumPlane.far);
		float texelSize = computeTexelWorldSize(linearDepth, currentFrustumPlane.near, currentFrustumPlane.nearTexelSizeWS);

		float3 offset = computeShadowNormalOffset(macroN, macroNoL, L, texelSize);
		float3 receiver = pos + offset;

		//Apply offset to light direction, and sample cubemap used updated direction
		currentDepth = computeLightDepth(receiver, VP, sm_UV);
		lookupDir = lookupCubemapFrom_UVFace(sm_UV, cubemapFace);
	}
	float visibility = shadowArray.SampleCmp(g_bilinearCmpClamp, float4(lookupDir , arrayIndex), currentDepth);
	return visibility;

	//float visibility = shadowArray.Sample(g_pointClamp, float4(lookupDir, arrayIndex));
	//return currentDepth > visibility;


}
#endif //_cbSHADOW_MAP_HLSL

