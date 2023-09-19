#include "../Registers.hlsli"
#include "../cbPerView.hlsli"
#include "../cbPerFrame.hlsli"
#include "../Common.hlsli"
#include "../Colors.hlsli"
#include "../BRDFs/Lambert_CookTorrance.hlsli"
#include "../SphereLights.hlsli"

struct VSOut
{
	float4 clip : SV_POSITION;
	nointerpolation float  radius : RADIUS;
	nointerpolation float  size : SIZE;
	nointerpolation float3 punctualPos : PPOSITION;
	nointerpolation float4 emission : EMISSION;
};

Texture2D<float>                g_depthBuffer         : register(srv_GB_DEPTH);
Texture2D<float3>               g_albedo              : register(srv_GB_ALBEDO);
Texture2D<float4>               g_normals             : register(srv_GB_NORMAL);
Texture2D<float2>               g_roughMetal          : register(srv_GB_ROUGH_METAL);

//GBUFFERS
float4 main(VSOut vout) : SV_TARGET
{

	//rebuild space
	float4 clipCoords;
	clipCoords.x = vout.clip.x * 2 * g_resolution.z - 1.0f;
	clipCoords.y = 1.0f - vout.clip.y * 2 * g_resolution.w;
	clipCoords.z = g_depthBuffer.Load(uint3(vout.clip.x, vout.clip.y, 0));
	clipCoords.w = 1.0f;

	clipCoords = mul(clipCoords, g_invP);
	clipCoords /= clipCoords.w;

	float3 scenePos;
#ifdef CAMERA_CENTER_WS
	row_major float3x3 cameraRotation = float3x3(g_invV[0].xyz, g_invV[1].xyz, g_invV[2].xyz);

	scenePos = mul(clipCoords.xyz, cameraRotation);

#else

#endif

	float distance;
	float3 relPos;
	float3 L = computeLightDirection(vout.punctualPos, scenePos, relPos, distance);

	float ratio = distance / vout.radius;
	float4 outColor = BLACK;

	
	if (ratio < 1.0f)
	{
		float3 albedo = g_albedo.Load(uint3(vout.clip.xy, 0));
		float  metalness = g_roughMetal.Load(uint3(vout.clip.xy, 0)).y;
		float3 microNormal = unpackOctahedron(g_normals.Load(uint3(vout.clip.xy, 0)));

		float3 diffuseColor = (1.0f - metalness) * albedo;
		float3 F0 = fresnel0(albedo, metalness);

		float NoL = max(0.0f, dot(L, microNormal));

		float distanceSq = distance * distance;
		float sizeSq = vout.size * vout.size;
		float radiusSq = vout.radius * vout.radius;
		float cosSolidAngle;
		float solidAngle = computeSphereSolidAngle(distanceSq, sizeSq, cosSolidAngle);
		//we want at distance = radius to have intensity ~0
		float3 irradiance = float3(0.01f, 0.01f, 0.01f);
		float3 radiance = radianceFromIrradiance(irradiance, radiusSq, sizeSq) * vout.emission.rgb;

		outColor.rgb = radiance * diffuseColor * _1DIVPI * (1.0f - F0) * NoL * solidAngle;
	}
	return outColor;
}