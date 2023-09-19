#include "Common.hlsli"
#include "sSamplers.hlsli"

#ifndef _SPHERE_LIGHTS_HLSL_
#define _SPHERE_LIGHTS_HLSL_
struct Light
{
	float3 radiance;
	float  solidAngle;

	float3 dir; //light vector that points to actual light pos
	float  distance;

	float  cos; //cos of aparent size(solid anlge)
	float  radius;
};//Note that distance, cos and radius fields are not used in directional sphere lights

struct DirectionalSphereLight
{
	float3   luminance;
	float    solidAngle;

	float3   direction;
};

struct PointSphereLight
{
	float3   luminance;
	float    radius;

	float3   position;
};

struct SpotSphereLight
{
	float3   luminance;
	float    radius;

	float3   position;
	float    angleScale;

	float3   orientation;
	float    angleOffset;

	row_major float4x4 viewProjection;

};

float3 computeLightDirection(in float3 lightPos, in float3 surfacePos, out float3 lightRelPos, out float distance)
{
	lightRelPos = lightPos - surfacePos;
	distance = length(lightRelPos);


	return lightRelPos / distance;
}

float computeSphereSolidAngle(in float squareDistance, in float squareRadius, out float cosSolidAngle)
{
	float ratio = min(squareRadius / squareDistance, 1.0f - EPSILON);

	cosSolidAngle = sqrt(1.0f - ratio);

	return _2PI * (1.0f - cosSolidAngle);
}

float computeAngleAttenuation(in float3 lightDirection, in float3 lightOrientation, in float angleScale, in float angleOffset)
{
	float cosineDot = max(dot(lightDirection, lightOrientation), 0.0f);
	float attenuation = saturate(cosineDot * angleScale + angleOffset);

	return attenuation * attenuation;
}



float3 radianceFromIrradiance(in float3 irradiance, in float squareDistance, in float squareRadius)
{
	float ratio = min(squareRadius / squareDistance, 1.0f - EPSILON);
	float den = 1.0f - sqrt(1.0f - ratio);
	return irradiance / den;
}

float horizonFalloffFactor(in float3 macronormal, in float lightRadius, in float3 micronormal, in float3 sphereRelPos)
{
	float h = max(dot(macronormal, sphereRelPos), -lightRadius);
	float macronormalFalloff = min(1.0f, (h + lightRadius) / (2.0f * lightRadius));

	h = max(dot(micronormal, sphereRelPos), -lightRadius);
	float micronormalFalloff = min(1.0f, (h + lightRadius) / (2.0f * lightRadius));

	return micronormalFalloff * macronormalFalloff;
}


// May return direction pointing beneath surface horizon (dot(N, dir) < 0), use clampDirToHorizon to fix it.
// sphereCos is cosine of light sphere solid angle.
// sphereRelPos is position of a sphere relative to surface:
// 'sphereDir == normalize(sphereRelPos)' and 'sphereDir * sphereDist == sphereRelPos'
float3 approximateClosestSphereDir(out bool intersects, float3 reflectionDir, float sphereCos,
    float3 sphereRelPos, float3 sphereDir, float sphereDist, float sphereRadius)
{
    float RoS = dot(reflectionDir, sphereDir);

    intersects = RoS >= sphereCos;
    if (intersects) return reflectionDir;
    if (RoS < 0.0) return sphereDir;

    float3 closestPointDir = normalize(reflectionDir * sphereDist * RoS - sphereRelPos);
    return normalize(sphereRelPos + sphereRadius * closestPointDir);
}

// Input dir and NoD is N and NoL in a case of lighting computation 
void clampDirToHorizon(inout float3 dir, inout float NoD, float3 normal, float minNoD)
{
	if (NoD < minNoD)
	{
		dir = normalize(dir + (minNoD - NoD) * normal);
		NoD = minNoD;
	}
}

// [ de Carpentier 2017, "Decima Engine: Advances in Lighting and AA" ]
void SphereMaxNoH(out bool intersects, in float NoV, inout float NoL, inout float VoL, float SinAlpha, float CosAlpha, bool bNewtonIteration, out float NoH, out float VoH)
{
	float RoL = 2 * NoL * NoV - VoL;
	if (RoL >= CosAlpha)
	{
		NoH = 1;
		VoH = abs(NoV);
		intersects = true;
	}
	else
	{
		float rInvLengthT = SinAlpha * rsqrt(1 - RoL * RoL + MIN_NoV_CLAMP);
		float NoTr = rInvLengthT * (NoV - RoL * NoL);
		float VoTr = rInvLengthT * (2 * NoV * NoV - 1 - RoL * VoL);

		if (bNewtonIteration && SinAlpha != 0)
		{
			// dot( cross(N,L), V )
			float NxLoV = sqrt(saturate(1 - pow(NoL, 2) - pow(NoV, 2) - pow(VoL, 2) + 2 * NoL * NoV * VoL));

			float NoBr = rInvLengthT * NxLoV;
			float VoBr = rInvLengthT * NxLoV * 2 * NoV;

			float NoLVTr = NoL * CosAlpha + NoV + NoTr;
			float VoLVTr = VoL * CosAlpha + 1 + VoTr;

			float p = NoBr * VoLVTr;
			float q = NoLVTr * VoLVTr;
			float s = VoBr * NoLVTr;

			float xNum = q * (-0.5 * p + 0.25 * VoBr * NoLVTr);
			float xDenom = p * p + s * (s - 2 * p) + NoLVTr * ((NoL * CosAlpha + NoV) * pow(VoLVTr, 2) + q * (-0.5 * (VoLVTr + VoL * CosAlpha) - 0.5));
			float TwoX1 = 2 * xNum / (pow(xDenom, 2) + pow(xNum, 2));
			float SinTheta = TwoX1 * xDenom;
			float CosTheta = 1.0 - TwoX1 * xNum;
			NoTr = CosTheta * NoTr + SinTheta * NoBr;
			VoTr = CosTheta * VoTr + SinTheta * VoBr;
		}

		NoL = NoL * CosAlpha + NoTr;
		VoL = VoL * CosAlpha + VoTr;

		intersects = false;
		float InvLenH = rsqrt(2 + 2 * VoL);
		NoH = saturate((NoL + NoV) * InvLenH);
		VoH = saturate(InvLenH + InvLenH * VoL);
	}
}

//returns light radiance after falloffs and other computations


Light sampleLight(in PointSphereLight inLight, in float3 surfacePos, in float3 N, out float NoL)
{
	float3 sphereRelPos;
	float sphereDist;
	float3 sphereDir = computeLightDirection(inLight.position, surfacePos, sphereRelPos, sphereDist);

	NoL = dot(sphereDir, N);

	float cosSolidAngle;
	float solidAngle = computeSphereSolidAngle(sphereDist * sphereDist, inLight.radius * inLight.radius, cosSolidAngle);

	Light outLight = { inLight.luminance, solidAngle, sphereDir, sphereDist, cosSolidAngle, inLight.radius };
	return outLight;
}

Light sampleLight(in SpotSphereLight inLight, in float3 surfacePos, in float3 N, in Texture2D<float1> masking, out float NoL)
{
	float3 sphereRelPos;
	float sphereDist;
	float3 sphereDir = computeLightDirection(inLight.position, surfacePos, sphereRelPos, sphereDist);

	NoL = dot(sphereDir, N);

	//Masking
	float4 clipSpotL = mul(float4(surfacePos, 1.0f), inLight.viewProjection);
	float2 maskUV = (clipSpotL.xy / clipSpotL.w + 1.0f) * 0.5f;
	float mask = masking.Sample(g_trilinearWrap, maskUV).r;
	mask = 1.0f;


	float cosSolidAngle;
	float solidAngle = computeSphereSolidAngle(sphereDist * sphereDist, inLight.radius * inLight.radius, cosSolidAngle);

	//Attenuation
	float angleAttenuation = computeAngleAttenuation(sphereDir, -inLight.orientation, inLight.angleScale, inLight.angleOffset);

	Light outLight = { angleAttenuation * mask * inLight.luminance, solidAngle, sphereDir, sphereDist, cosSolidAngle, inLight.radius };

	return outLight;
}




#endif //_SPHERE_LIGHTS_HLSL