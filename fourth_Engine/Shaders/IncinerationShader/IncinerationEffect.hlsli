#include "../Common.hlsli"
#include "../Particles/gpuParticles.hlsli"

#ifndef _INCINERATION_SHADER_HLSLI_
#define _INCINERATION_SHADER_HLSLI_

//-------------VERTEX SHADER--------------------//
struct IESphereParams
{
	float  vertexRadius;
	float  currentRadius;
	float  prevRadius;
	float  maxRadius;
};

//This function will checks wether this vertex should spawn a particle or not. A vertex will be spawned if
// its position is inside of the spawning sphere and its VtxID/Index is a multiple of X
bool IEcheckSpawn(
	in IESphereParams sphere,
	in uint vtxID,
	in float multiple)
{
//all threads will resolve to the same branch
	if (sphere.currentRadius > sphere.maxRadius)
		return false;

	float mult = max(1.0f, multiple);

	float mod = fmod(vtxID, mult);
	bool isMultiple = mod == 0;

	if (!isMultiple)
		return false;

	return sphere.prevRadius < sphere.vertexRadius && 
		                         sphere.vertexRadius < sphere.currentRadius;
}


//------------PIXEL SHADER-------------------//

static const float EDGE_FALLOFF = 0.15f;

struct IESampleNoise
{
	Texture2D<float1> noise;
	sampler           noiseSampler;
};

float IE_DiscardSurface(in float surfaceRadius, in float currentRadius, in float prevRadius, in float2 uv, IESampleNoise tex)
{
	float threshold = (currentRadius - prevRadius) * EDGE_FALLOFF + (prevRadius - EDGE_FALLOFF);//threshold radius Squared

	if (surfaceRadius < threshold)
		discard;

	//push radius by the threshold to give some thickness

	//float visibility = max( 1.0f, 1.0f / (currentRadiusSq - threshold) * (surfaceRadiusSq - currentRadiusSq) + 1.0f );
	float visibility = saturate((surfaceRadius - threshold) / (currentRadius - threshold));
	float noise = smoothstep(0.1f, 0.9f, tex.noise.Sample(tex.noiseSampler, uv));

	if (noise > visibility)
		discard;

	return visibility;
}

float3 IE_computeEmission(in float visibility, in float multiplier, in float3 baseEmission)//this function might discard a pixel
{
	static const float euler = 2.71828f;
	float A = multiplier / (1.0f - euler);

	float invVis = 1.0f - visibility;

	return baseEmission * A *( 1.0f - pow(euler, invVis));
}



#endif//_INCINERATION_SHADER_HLSLI