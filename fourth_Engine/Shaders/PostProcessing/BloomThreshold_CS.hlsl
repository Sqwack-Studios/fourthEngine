#include "../Common.hlsli"
#include "../cbPerFrame.hlsli"
#include "Bloom.hlsli"

Texture2D<float4> source_HDR : register(SRV_PP_0);
RWTexture2D<float4> dest_HDR : register(UAV_PP_COMPUTE_0);


cbuffer immediateBloom : register(cb_DS_BLOOM)
{
	float2 g_bloomResolution;
	float  g_bottomThreshold;
	float  g_upThreshold;

	int    g_mode;
	float  pad[3];
};



[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	const float2 texel = DTid.xy;
	const float2 c_texel = texel + 0.5f;

	float3 color = float3(0.0f, 0.0f, 0.0f);
	if (g_mode == MODE_DOWNSAMPLE)
	{
		color = downsampleFirstPass(g_bilinearClamp, source_HDR, c_texel, g_bloomResolution);
	}

	if (g_mode == MODE_UPSAMPLE)
	{
		float2 uv = c_texel * float2(g_bottomThreshold, g_upThreshold);
		color = upsampleBloom(g_bilinearClamp, source_HDR, uv, 1.0f, g_bloomResolution);
	}

	dest_HDR[DTid.xy] = float4(color, 1.0f);
}