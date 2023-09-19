#include "../Common.hlsli"
//Things noted, things to ask Emil
//1- Normalization should happen to all of the BRDF, not just specular term. I've readen that taking into account diffuse part makes it harder for artists
//2- If we are taking into account energy conservation, then:
// 2.a - Specularity must be a float3 instead of a single float, because energy is not absorbed equally. 
// 2.b - Albedo and specularity must be related: A + S <= 1. A fully specular material is just reflective, i.e, a mirror. Albedo is zero.
//

//Roughness remapping from [0, 1] to [1000, 0]
float remapRoughness(in float roughness)
{
	return pow(2, (1.0f - roughness) * 10);
}

//Linear fit of specular term normalization from n [0, 1000].
float normalizeSpecular(float specularPower)
{
	//(8PI *(exp2(-n/2) + n) / ((n + 2)(n + 4)) )^-1
	return 0.0397814256606231f * specularPower + 0.244365698163161f;
}

float BlinnPhongDiffuseContribution()
{
	return _1DIVPI;
}

float BlinnPhongSpecularContribution(in float specularPower, in float specularity, in float3 normal, in float3 viewDirection, in float3 lightDirection)
{
	float3 halfVector = normalize(lightDirection + viewDirection);
	float normalizationFactor = normalizeSpecular(specularPower);

	float dotNH = clamp(dot(normal, halfVector), 0.0f, 1.0f);

	return specularity * pow(dotNH, specularPower);
}

