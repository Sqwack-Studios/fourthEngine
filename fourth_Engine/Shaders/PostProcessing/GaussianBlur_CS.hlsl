#include "../Common.hlsli"
#include "Bloom.hlsli"

Texture2D<float4> prevMipSource : register(SRV_PP_0);
Texture2D<float4> compositeSource : register(SRV_PP_1);
RWTexture2D<float4> currentMipTarget : register(UAV_PP_COMPUTE_0);

groupshared float3 color_cache[CACHE_SIZE];

cbuffer immediateBloom : register(cb_DS_BLOOM)
{
	float4 g_resolution;

	int    g_flags;
	float  pad[3];
};


[numthreads(GAUSSIAN_BLUR_THREADCOUNT, 1, 1)]
void main( uint3 Gid : SV_GroupID, uint groupIndex : SV_GroupIndex )
{
	const bool horizontal = (g_flags & FLAG_GAUSSIAN_HORIZONTAL);
	const bool composite = (g_flags & FLAG_GAUSSIAN_COMPOSITE);

	uint2 tileStart = Gid.xy;

	float2 direction;
	if (horizontal)
	{
		direction = float2(1.0f, 0.0f);
		tileStart.x *= GAUSSIAN_BLUR_THREADCOUNT;
	}
	else
	{
		direction = float2(0.0f, 1.0f);
		tileStart.y *= GAUSSIAN_BLUR_THREADCOUNT;
	}

	int i;
	for (i = groupIndex; i < CACHE_SIZE; i += GAUSSIAN_BLUR_THREADCOUNT)
	{
		float2 webo = direction * (i - TILE_BORDER);
		const float2 uv = (tileStart + float2(0.5f, 0.5f) + webo ) * g_resolution.zw;
		color_cache[i] = prevMipSource.SampleLevel(g_pointClamp, uv, 0).rgb;
	}

	GroupMemoryBarrierWithGroupSync();

	const int2 targetPixel = tileStart + groupIndex * direction;

	if (targetPixel.x >= g_resolution.x || targetPixel.y >= g_resolution.y)
	{
		return;
	}

	float3 color = float3(0.0f, 0.0f, 0.0f);
	const uint center = TILE_BORDER + groupIndex;

	for (i = 0; i < GAUSSIAN_KERNEL; ++i)
	{
		const uint sam = center + gaussianOffsets[i];
		const float3 cachedColor = color_cache[sam];

		color += cachedColor * gaussianWeightsNormalized[i];
	}

	if (composite)
	{
		color += compositeSource.Load(int3(targetPixel, 0)).rgb;
		color *= 0.5f;
	}
	
	currentMipTarget[targetPixel] = float4(color, 1.0f);
}

