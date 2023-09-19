#include "IncinerationEffect.hlsli"
#include "../OpaqueShader/OpaqueShader.hlsli"
#include "../Colors.hlsli"
#include "../sSamplers.hlsli"
#include "../cbPerFrame.hlsli"

//MATERIALS
Texture2D<float4> g_albedoMap : register(srv_ALBEDO);
Texture2D<float3> g_normalMap : register(srv_NORMAL);
Texture2D<float1> g_roughMap  : register(srv_ROUGH);
Texture2D<float1> g_metalMap  : register(srv_METAL);
Texture2D<float1> g_incNoise  : register(srv_INC_NOISE);



cbuffer PerMaterial : register(cb_MATERIAL)
{
	uint     g_matFlags;
	float    g_roughness;
	float    g_metalness;
	float    padding[1];
};

struct VSOut
{
	float4 clip                           : SV_POSITION;
	float3 pos                            : POSITION;
	float3 normal                         : NORMAL;
	float2 uv                             : UV;
	nointerpolation float3  spherePos     : SPHEREPOS;
	nointerpolation uint objectID         : OBJECTID;
	float3 tangent                        : TANGENT;
	nointerpolation float currentRadius : CRADIUS;
	float3 bitangent                      : BITANGENT;
	nointerpolation float prevRadius      : PRADIUS;
	nointerpolation float3 emission          : EMISSION;
};

struct PSOut
{
	float3 albedo      : TARGET_ALBEDO;
	float2 rough_metal : TARGET_ROUGH_METAL;
	float4 normals     : TARGET_TX_GM_NORMAL;
	uint   objectID    : TARGET_OBJECTID;
	float3 emission    : TARGET_EMISSION;
};


PSOut main(VSOut vout) 
{
	float visibility; 
	float distance = length(vout.spherePos - vout.pos);
	{
		IESampleNoise texNoise;
		texNoise.noise = g_incNoise;
		texNoise.noiseSampler = g_trilinearWrap;
		visibility = IE_DiscardSurface(distance, vout.currentRadius, vout.prevRadius, vout.uv, texNoise);
	}

	static const float EMISSION_MULTIPLIER = 2.0f;

	float3 incEmission = IE_computeEmission(visibility, EMISSION_MULTIPLIER, vout.emission);

	row_major float3x3 TBN = float3x3(vout.tangent, vout.bitangent, vout.normal);

	OpaqueMaterial opaqueMat;
	opaqueMat.albedo = g_albedoMap;
	opaqueMat.normal = g_normalMap;
	opaqueMat.rough = g_roughMap;
	opaqueMat.metal = g_metalMap;
	opaqueMat.flags = g_matFlags;
	opaqueMat.auxAlbedo = MAGENTA;
	opaqueMat.auxNormal = BLUE.rgb;
	opaqueMat.auxRoughness = g_roughness;
	opaqueMat.auxMetalness = g_metalness;

	OpaqueTexSamplers opaqueSamplers = { g_anisotropicWrap };

	GBufferOut gbOut = opaqueShader(vout.pos, TBN, vout.uv, opaqueMat, opaqueSamplers);

	
	PSOut psout;
	psout.albedo = gbOut.albedo;
	psout.normals = gbOut.normals;
	if (g_overwriteRoughness)
	{
		psout.rough_metal.x = g_overwrittenRoughness;
		psout.rough_metal.y = gbOut.rough_metal.y;
	}
	else
	{
		psout.rough_metal = gbOut.rough_metal;
	}
	psout.objectID = vout.objectID;
	psout.emission = gbOut.emission + incEmission.rgb;

	return psout;
}