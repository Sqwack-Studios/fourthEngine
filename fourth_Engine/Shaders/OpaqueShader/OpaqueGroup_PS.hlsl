#include "../Registers.hlsli"
#include "../Colors.hlsli"
#include "OpaqueShader.hlsli"

//Materials
Texture2D<float4> g_albedoMap : register(srv_ALBEDO);
Texture2D<float3> g_normalMap : register(srv_NORMAL);
Texture2D<float1> g_roughMap  : register(srv_ROUGH);
Texture2D<float1> g_metalMap  : register(srv_METAL);

cbuffer PerMaterial : register(cb_MATERIAL)
{
	//Bools are 4 bytes in hlsl. Considering that small buffers might be performance critical, use a single int to store booleans and perform bitwise operations to extract data?
	uint g_materialFlags;
	float g_roughness;
	float g_metalness;
};


struct VSOut
{
	float4 clip        : SV_POSITION;
	float3 pos         : POSITION;
	float3 normal      : NORMAL;
	float3 tangent     : TANGENT;
	float3 bitangent   : BITANGENT;
	float2 uv          : UV;
	nointerpolation uint objectID : ID;
};


struct PS_OUT
{
	float3 albedo      : TARGET_ALBEDO;
	float2 rough_metal : TARGET_ROUGH_METAL;
	float4 normals     : TARGET_TX_GM_NORMAL;
	uint   objectID    : TARGET_OBJECTID;
	float3 emission    : TARGET_EMISSION;
};

PS_OUT main(VSOut vout)
{
	row_major float3x3 TBN = float3x3(vout.tangent, vout.bitangent, vout.normal);

	OpaqueMaterial opaqueMaterial;
	opaqueMaterial.albedo = g_albedoMap;
	opaqueMaterial.normal = g_normalMap;
	opaqueMaterial.rough  = g_roughMap;
	opaqueMaterial.metal  = g_metalMap;
	opaqueMaterial.flags  = g_materialFlags;
	opaqueMaterial.auxAlbedo = MAGENTA;
	opaqueMaterial.auxNormal = BLUE.rgb;
	opaqueMaterial.auxRoughness = g_roughness;
	opaqueMaterial.auxMetalness = g_metalness;



	OpaqueTexSamplers opaqueSamplers = { g_anisotropicWrap };

	GBufferOut gbOut = opaqueShader(vout.pos, TBN, vout.uv, opaqueMaterial, opaqueSamplers);

	if (g_overwriteRoughness)
		gbOut.rough_metal.x = g_overwrittenRoughness;

	PS_OUT psout;
	psout.albedo = gbOut.albedo;
	psout.rough_metal = gbOut.rough_metal;
	psout.normals = gbOut.normals;
	psout.objectID = vout.objectID;
	psout.emission = gbOut.emission;

	return psout;
}