#include "HemisphereSampling.hlsli"



float3 computeIBLDiffuseIrradiance(in float3 hemisphere, in uint samples, in uint txRes, uniform TextureCube cubemap)
{
	//compute frisvad's 3x3 rotation matrix
	const float3x3 rotate = basisFromDir(hemisphere);

	float3 integralSum = 0.0f;
	//constant for uniform sampling
    float mipLevel = hemisphereMip( 1 / samples, txRes);

	//generate a light sampling direction, rotate it
	for (uint i = 0; i < samples; ++i)
	{
		float NoL;
		float3 L = mul(randomHemisphere(NoL, i, samples), rotate);

		integralSum += (cubemap.SampleLevel(g_pointWrap, L, mipLevel).rgb * NoL * (1.0f - fresnel(NoL, 0.04f)));
	}

	return  integralSum * (2.0f / samples);
}

float3 computeIBLSpecularIrradiance(in float3 hemisphere, in uint samples, in float roughness2, in uint txRes, uniform TextureCube cubemap)
{
	row_major float3x3 rotate = basisFromDir(hemisphere);
	float3 integralSum = 0.0f;

	for (uint i = 0; i < samples; ++i)
	{
		float NoH;
		float3 H = randomGGX(NoH, i, samples, roughness2, rotate);
		float3 N = hemisphere;
		float3 V = N;
		float3 L = reflect(-hemisphere, H);

	    float sampleProb = importanceGGXSampleSize(samples, NoH, roughness2);
	    float mipLevel = hemisphereMip(sampleProb, txRes);

		float NoL = dot(N, L);
		
		if (NoL > MIN_NoV_CLAMP)
		{
			integralSum += cubemap.SampleLevel(g_pointWrap, L, mipLevel).rgb;
		}
	}

	return integralSum / samples;
}

float2 computeIBLSpecularReflectance(in uint samples, in float roughness, in float NoV)
{
    const float3 N = float3(0.0f, 0.0f, 1.0f);

	float rough2 = roughness * roughness;
	float rough4 = max(rough2 * rough2, 0.001f);


	float Red = 0.0f;
	float Green = 0.0f;

	for (uint i = 0; i < samples; ++i)
	{
		float3 H = randomGGX(randomHammersley(i, samples), rough4);
		float3 V = float3(sqrt(1.0f - NoV * NoV), 0.0f, NoV);
		float3 L = reflect(-V, H);

		float NoH = max(H.z, MIN_NoV_CLAMP);
		float NoL = max(L.z, MIN_NoV_CLAMP);
		float HoV = max(dot(H, V), MIN_NoV_CLAMP);

		if (NoL > MIN_NoV_CLAMP)
		{

			float G = smith(rough4, NoV, NoL);
			float inv_NoV_mul_NoH = 1.0f / (NoV * NoH);

			float G_Vis = G * HoV * inv_NoV_mul_NoH;

			float fresnelC = pow(1.0f - HoV, 5);

			Red += G_Vis * (1.0f - fresnelC);
			Green += G_Vis * fresnelC;

		}

	}

	return float2(Red, Green) / samples;
}