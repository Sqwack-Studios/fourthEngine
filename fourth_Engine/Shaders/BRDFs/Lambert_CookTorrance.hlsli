#include "../Common.hlsli"

#ifndef _LAMBERT_COOKTORRANCE_BRDF_HLSL
#define _LAMBERT_COOKTORRANCE_BRDF_HLSL


struct Surface
{
	float3 diffuseColor;
	float  roughness4;

	float3 normal;
	float  roughnessLinear;

	float3 F0;
};

struct View
{
	float3 viewDir;
	//This is reflection from MICRONORMAL
	float3 reflectionDir;
	float  NoV;
};


float3 fresnel0(in float3 albedo, in float metalness)
{
	return lerp(0.04f, albedo, metalness);
}

float lambert()
{
	return _1DIVPI;
}

// Schlick's approximation of Fresnel reflectance,
float3 fresnel(float NoL, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - NoL, 5);
}

// Height-correlated Smith G2 for GGX,
// Filament, 4.4.2 Geometric shadowing
// rough4 is initial roughness value in power of 4
float smith(float rough4, float NoV, float NoL)
{
	NoV *= NoV;
	NoL *= NoL;
	return 2.0 / (sqrt(1 + rough4 * (1 - NoV) / NoV) + sqrt(1 + rough4 * (1 - NoL) / NoL));
}

// GGX normal distribution,
// Real-Time Rendering 4th Edition, page 340, equation 9.41
// rough4 is initial roughness value in power of 4
float ggx(float rough4, float NoH)
{
	float denom = NoH * NoH * (rough4 - 1.0) + 1.0;
	denom = _PI * denom * denom;
	return rough4 / denom;
}

void L_CT_BRDF(in Surface s, in View v, in float3 L, in float NoL, in float solidAngle, out float3 diffuse, out float3 specular)
{
	float3 h = normalize(v.viewDir + L);


	float NoH = max(dot(s.normal, h), MIN_NoV_CLAMP);
	float HoL = dot(h, L); // always positive because H is half angle V-L

	float NoV = abs(v.NoV);

	float D_GGX = ggx(s.roughness4, NoH);
	float G_GGX = smith(s.roughness4, NoV, NoL);


	float3 fD = solidAngle * s.diffuseColor * lambert() * (1.0f - fresnel(NoL, s.F0)) * NoL;
	float3 fS = min(1.0f, (D_GGX * solidAngle) / (4.0f * NoV)) * G_GGX * fresnel(HoL, s.F0);

	diffuse = fD;
	specular = fS;
}

void L_CT_BRDF(in float NoL, in float NoV, in float NoH, in float HoL, in Surface s, in float solidAngle, out float3 diffuse, out float3 specular)
{
	float D = ggx(s.roughness4, NoH);
	float G = smith(s.roughness4, abs(NoV), NoL);

	float3 fD = solidAngle * s.diffuseColor * lambert() * (1.0f - fresnel(NoL, s.F0)) * NoL;
	float3 fS = min(1.0f, (D * solidAngle) / (4.0f * abs(NoV))) * G * fresnel(HoL, s.F0);

	diffuse = fD;
	specular = fS;
}
#endif //_LAMBERT_COOKTORRANCE_BRDF_HLSL