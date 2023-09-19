#include "../sSamplers.hlsli"
#include "../cbLights.hlsli"
#include "../cbPerView.hlsli"

struct VSOut
{
	float4 clip                         : SV_POSITION;
    float3 worldPos                     : POSITION;
	float2 uv                           : UV;

	nointerpolation float  timeFraction : TIME;
	nointerpolation float3   up         : UP;
	nointerpolation float3   forward    : FORWARD;
	nointerpolation uint     tileIdx    : TILE;
	nointerpolation float4 color        : COLOR;
};

Texture2D<float3>  g_particleRLU : register(srv_LM_RLU);
Texture2D<float3>  g_particleDBF : register(srv_LM_DBF);
Texture2D<float4>  g_particleEMVA : register(srv_LM_EMVA);
Texture2D<float>   g_depthBuffer  : register(srv_GB_DEPTH);
Texture2D<float1>  g_spotMask     : register(srv_LIGHTMASK);

static const float emissionMultiplier = 100.0f;
static const float3 emissionColor = float3(0.917f, 0.137f, 0.0f) * emissionMultiplier;
static const float  smokeThickness = 0.20f; //20cm thickness

cbuffer tiledAtlasDesc : register(cb_SMOKE_TILED_ATLAS)
{
	uint g_atlasWidth, g_atlasHeight, g_numTiles, g_tileWidth, g_tileHeight, g_maxTilesPerRow, g_maxTilesPerCol;
}

float3 computeLightmapContribution(in float3 lightPower, in float3 L, in float3 lightmapRLU, in float3 lightmapDBF, in float3 right, in float3 up, in float3 forward)
{
	float3 accumulate = float3(0.0f, 0.0f, 0.0f);

	accumulate.rgb += lightPower * lightmapRLU.x * max(0.0f, dot(right, L));
	accumulate.rgb += lightPower * lightmapRLU.y * max(0.0f, dot(-right, L));
	accumulate.rgb += lightPower * lightmapRLU.z * max(0.0f, dot(up, L));
	accumulate.rgb += lightPower * lightmapDBF.x * max(0.0f, dot(-up, L));
	accumulate.rgb += lightPower * lightmapDBF.y * max(0.0f, dot(forward, L));
	accumulate.rgb += lightPower * lightmapDBF.z * max(0.0f, dot(-forward, L));

	return accumulate;
}

void computeTilePositionFromIndex(in uint index, out float xpos, out float ypos)
{
	uint tileCol, tileRow;

	tileRow = index / g_maxTilesPerCol;
	tileCol = uint(fmod((index), g_maxTilesPerCol));

	xpos = (g_tileWidth)*tileCol + 0.5f;
	ypos = (g_tileHeight)*tileRow + 0.5f;
}

float4 main(VSOut vout) : SV_TARGET
{

	float2 currentPos, nextPos;
	computeTilePositionFromIndex(vout.tileIdx, currentPos.x, currentPos.y);
	computeTilePositionFromIndex(vout.tileIdx + 1, nextPos.x, nextPos.y);

	float2 tileUVSize = float2(float(g_tileWidth) / float(g_atlasWidth), float(g_tileHeight) / float(g_atlasHeight));

	float2 currentUV = float2(currentPos.x / float(g_atlasWidth), currentPos.y / float(g_atlasHeight)) + vout.uv * tileUVSize;
	float2 nextUV = float2(nextPos.x / float(g_atlasWidth), nextPos.y / float(g_atlasHeight)) + vout.uv * tileUVSize;


	float2 mv0 = 2.0f * g_particleEMVA.Sample(g_trilinearWrap, currentUV).gb - 1.0f;
	float2 mv1 = 2.0f * g_particleEMVA.Sample(g_trilinearWrap, nextUV).gb - 1.0f;

	mv0.y = -mv0.y;
	mv1.y = -mv1.y;

	static const float MV_SCALE = 0.0015f;
	float time = vout.timeFraction;

	float2 uv0 = currentUV;
	float2 uv1 = nextUV;

	uv0 -= mv0 * MV_SCALE * time;
	uv1 -= mv1 * MV_SCALE * (time - 1.0f);


	float2 emissionAlpha0 = g_particleEMVA.Sample(g_trilinearWrap, uv0).ra;
	float2 emissionAlpha1 = g_particleEMVA.Sample(g_trilinearWrap, uv1).ra;

	// .x - right, .y - left, .z - up
	float3 lightmapRLU0 = g_particleRLU.Sample(g_trilinearWrap, uv0).rgb;
	float3 lightmapRLU1 = g_particleRLU.Sample(g_trilinearWrap, uv1).rgb;

	// .x - down, .y - back, .z - front
	float3 lightmapDBF0 = g_particleDBF.Sample(g_trilinearWrap, uv0).rgb;
	float3 lightmapDBF1 = g_particleDBF.Sample(g_trilinearWrap, uv1).rgb;

	// ----------- lerp values -----------

	float2 emissionAlpha = lerp(emissionAlpha0, emissionAlpha1, time);
	float3 lightmapRLU = lerp(lightmapRLU0, lightmapRLU1, time);
	float3 lightmapDBF = lerp(lightmapDBF0, lightmapDBF1, time);

	float4 finalColor = vout.color;
	finalColor.a *= emissionAlpha.y;

	float3 emission = emissionAlpha.x * emissionColor;
	finalColor.rgb += emission;


	const float3 right = cross(vout.up, vout.forward);

	{
		DirectionalSphereLight directLight = g_directionalLight;

		float3 lightPower = directLight.luminance * directLight.solidAngle;
		float3 L = directLight.direction;

		finalColor.rgb += computeLightmapContribution(lightPower, L, lightmapRLU, lightmapDBF, right, vout.up, vout.forward);
	}

	{
		SpotSphereLight spotLight = g_spotLight;
		float NoL;
		Light light = sampleLight(spotLight, vout.worldPos, vout.forward, g_spotMask, NoL);

		float3 lightPower = light.radiance * light.solidAngle;
		float3 L = -light.dir;

		finalColor.rgb += computeLightmapContribution(lightPower, L, lightmapRLU, lightmapDBF, right, vout.up, vout.forward);
	}

	[unroll]
	for (uint pointIdx = 0; pointIdx < g_numPointLights; ++pointIdx)
	{
		PointSphereLight pointLight = g_pointLights[pointIdx];

		float NoL;
		Light light = sampleLight(pointLight, vout.worldPos, vout.forward, NoL);

		float3 lightPower = light.radiance * light.solidAngle;
		float3 L = -light.dir;

		finalColor.rgb += computeLightmapContribution(lightPower, L, lightmapRLU, lightmapDBF, right, vout.up, vout.forward);
	}

	//depth tests

	float opaqueDepth = linearizeDepth(g_depthBuffer.Load(uint3(vout.clip.x, vout.clip.y, 0)), g_nearPlane, g_farPlane);
	float currentDepth = linearizeDepth(vout.clip.z, g_nearPlane, g_farPlane);

	float opaqueOffset = saturate( (opaqueDepth - currentDepth) / smokeThickness);
	float proximityOffset = saturate((currentDepth - g_nearPlane * 10.0f ) / smokeThickness);

	float alphaFade = lerp(0.0f, 1.0f, opaqueOffset * proximityOffset);

	finalColor.a *= alphaFade;
	return finalColor;
}