#include "../Registers.hlsli"
#include "../Colors.hlsli"
#include "../OpaqueShader/OpaqueShader.hlsli"

//MATERIALS
Texture2D<float4> g_albedoMap : register(srv_ALBEDO);
Texture2D<float3> g_normalMap : register(srv_NORMAL);
Texture2D<float1> g_roughMap  : register(srv_ROUGH);
Texture2D<float1> g_metalMap  : register(srv_METAL);


cbuffer PerMaterial : register(cb_MATERIAL)
{
	//Bools are 4 bytes in hlsl. Considering that small buffers might be performance critical, use a single int to store booleans and perform bitwise operations to extract data?
	uint g_materialFlags;
	float g_materialRoughness;
	float g_materialMetalness;
};

//DISSOLUTION EFFECT
Texture2D<float1> g_dissolutionNoise : register(srv_DIS_NOISE);

#include "DissolutionEffect.hlsli"




struct VSOut
{
	float4 clip        : SV_POSITION;
	float3 pos         : POSITION;
	nointerpolation uint   objectID    : ID;
    float3 normal      : NORMAL;
	float3 tangent     : TANGENT;
	float3 bitangent   : BITANGENT;
	float2 uv          : UV;
	float2 time        : TIME;
};

struct PS_OUT
{
	float3 albedo : TARGET_ALBEDO;
	float2 rough_metal : TARGET_ROUGH_METAL;
	float4 normals  : TARGET_TX_GM_NORMAL;
	uint   objectID : TARGET_OBJECTID;
	float3 emission : TARGET_EMISSION;
};

static const float emissionMultiplier = 100.0f;
static const float3 emissionColor = float3(0.917f, 0.137f, 0.0f) * emissionMultiplier;
static const float  emissionTimeFraction = 0.05f;

PS_OUT main(VSOut vout)
{
	float timeScale = vout.time.x;
	float currentTime = vout.time.y;

	float scaledNoise = sampleNoise(vout.uv, timeScale, currentTime);
	bool shouldDiscard = currentTime < scaledNoise;

    if (shouldDiscard)
    {
    	discard;
    }

	float emissionTime = timeScale * emissionTimeFraction;
	float timeDiff = (currentTime - scaledNoise);
	float emissionFactor = saturate(1.0f - timeDiff / emissionTime);
	float3 emission = emissionColor * emissionFactor;

	row_major float3x3 TBN = float3x3(vout.tangent, vout.bitangent, vout.normal);

	OpaqueMaterial opaqueMaterial;
	opaqueMaterial.albedo        = g_albedoMap;
	opaqueMaterial.normal        = g_normalMap;
	opaqueMaterial.rough         = g_roughMap;
	opaqueMaterial.metal         = g_metalMap;
	opaqueMaterial.flags         = g_materialFlags;
	opaqueMaterial.auxAlbedo     = MAGENTA;
	opaqueMaterial.auxNormal     = BLUE.rgb;
	opaqueMaterial.auxRoughness  = g_materialRoughness;
	opaqueMaterial.auxMetalness  = g_materialMetalness;

	OpaqueTexSamplers opaqueSamplers = { g_anisotropicWrap };

	GBufferOut gbOut = opaqueShader(vout.pos, TBN, vout.uv, opaqueMaterial, opaqueSamplers);

	if (g_overwriteRoughness)
		gbOut.rough_metal.x = g_overwrittenRoughness;


	PS_OUT psOut = { gbOut.albedo, gbOut.rough_metal, gbOut.normals, vout.objectID, emission };

	return psOut;


	//float sampledNoise = sampleNoise(vout.uv, vout.time.x, vout.time.y);

	//float alphaFactor = 1.0f - sampledNoise / vout.time.y;
	//float threshold = 0.0f;
	//float falloff = 1.0f;
	//float alpha = computeAlphaToCoverage(alphaFactor, threshold, falloff);

	//float3 emissionColor = float3(1.0f, 0.5f, 0.0f) * scaledNoise;
	//float3 opaqueColor = opaqueShader(vout.pos, vout.normal, vout.tangent, vout.bitangent, vout.uv).rgb;

	//float lerpFactor = currentTime / timeScale;
	//float3 finalColor = lerp(emissionColor, opaqueColor, lerpFactor);

	//return float4(finalColor, 1.0f);

	//float3 opaqueColor = opaqueShader(vout.pos, vout.normal, vout.tangent, vout.bitangent, vout.uv).rgb;


	//return float4(opaqueColor, alpha);
}