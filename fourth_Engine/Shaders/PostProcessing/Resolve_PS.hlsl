#include "Resolve.hlsli"
#include "../cbPerFrame.hlsli"
#include "../Common.hlsli"
Texture2D<float4> g_hdrTx : register(srv_HDR_RESOLVE);
Texture2D<float4> g_bloomTx : register(srv_BLOOM_RESOLVE);
//Texture2DMS<float4> g_hdrTx : register(srv_ALBEDO);

struct PSIn
{
	float4 pos : SV_POSITION;
};


//We have to load each subsample, make exposure and tone corrections and then average them
//We can use GetDimensions to know how many subsamples we need to sample. Also we could just upload this as a resource in per frame buffer and thats it.

cbuffer immediateResolve : register(cb_PP_RESOLVE)
{
	uint g_flags;
}

static const uint HAS_BLOOM = 1;

float4 main(PSIn pin) : SV_TARGET
{

	float3 accumulateLDR_color = float3(0.0f, 0.0f, 0.0f);


	float3 hdrColor = g_hdrTx.Load(int3(pin.pos.xy, 0)).rgb;

	bool applyBloom = g_flags == HAS_BLOOM;

	if (applyBloom)
	{
		float3 bloomColor = g_bloomTx.SampleLevel(g_bilinearClamp, (pin.pos.xy + float2(0.5f, 0.5f))* g_resolution.zw, 0).rgb;

		bloomColor += g_bloomTx.SampleLevel(g_bilinearClamp, (pin.pos.xy + float2(-0.5f, 0.5f) ) * g_resolution.zw, 0).rgb;
		bloomColor += g_bloomTx.SampleLevel(g_bilinearClamp, (pin.pos.xy + float2(-0.5f, -0.5f) ) * g_resolution.zw, 0).rgb;
		bloomColor += g_bloomTx.SampleLevel(g_bilinearClamp, (pin.pos.xy + float2(0.5f, -0.5f) ) * g_resolution.zw, 0).rgb;
		bloomColor *= 0.25f;

		hdrColor = lerp(hdrColor, bloomColor, 0.03f);
		//float3 coco = max(hdrColor - bloomColor, float3(0.0f, 0.0f, 0.0f));
		//hdrColor += coco;
	}

	float3 exposureCorrected = adjustExposure(hdrColor, g_EV100);
	float3 toneMapped = acesHdr2Ldr(exposureCorrected);

	accumulateLDR_color += toneMapped;
	


	//for (uint subsample = 0; subsample < g_mainRTVSubsamples; ++subsample)
	//{
	//	float3 hdrColor = g_hdrTx.Load(int2(pin.pos.xy), subsample).rgb;
	//	float3 exposureCorrected = adjustExposure(hdrColor, g_EV100);
	//	float3 toneMapped = acesHdr2Ldr(exposureCorrected);

	//	accumulateLDR_color += toneMapped;
	//}
	//accumulateLDR_color /= g_mainRTVSubsamples;

	float bright = brightness(accumulateLDR_color);
	if (g_validateLuminance)
	{
	    float3 brightnessMap = luminanceValidation(accumulateLDR_color);
		accumulateLDR_color = brightnessMap;
	}


	float3 gammaCorrected = correctGamma(accumulateLDR_color, 2.2f);


	return float4(gammaCorrected.rgb, 1.0f);
}