#include "../sSamplers.hlsli"
#include "../BRDFs/Lambert_CookTorrance.hlsli"
#include "../cbPerFrame.hlsli"
#include "../cbLights.hlsli"
#include "../cbPerView.hlsli"
#include "../ShadowMapping/cbShadowMap.hlsli"

#ifndef _OPAQUE_SHADER_HLSLI_
#define _OPAQUE_SHADER_HLSLI_
//This shader needs:-------
// 
////MATERIALS
//Texture2D<float4> g_albedoMap : register(srv_ALBEDO);
//Texture2D<float3> g_normalMap : register(srv_NORMAL);
//Texture2D<float1> g_roughMap  : register(srv_ROUGH);
//Texture2D<float1> g_metalMap  : register(srv_METAL);
//
//cbuffer PerMaterial : register(cb_MATERIAL)
//{
//	//Bools are 4 bytes in hlsl. Considering that small buffers might be performance critical, use a single int to store booleans and perform bitwise operations to extract data?
//	uint g_materialFlags;
//	float g_roughness;
//	float g_metalness;
//};
//1st bit : checks normals
//2nd bit : checks roughness
//3rd bit : checks metalness
//4th bit : checks invert Y normals channel
//5th bit : checks build normal blue channel




struct GBufferOut
{
	float3   albedo;
	float4   normals;//XY octahedron packed microNormal. ZW octahedron packed macroNormal
	float2   rough_metal;
	float3   emission;
};

struct OpaqueMaterial
{
	Texture2D<float4> albedo;
	Texture2D<float3> normal;
	Texture2D<float1> rough;
	Texture2D<float1> metal;

	float4 auxAlbedo;
	float3 auxNormal;
	float  auxRoughness;
	float  auxMetalness;
	uint   flags;
};
//opaqueFlags:
//0: hasNormals
//1: hasRoughness
//2: hasMetalness
//3: invertY normal channel
//4: build blue channel
//
// 
// 
//
struct OpaqueTexSamplers
{
	sampler generalSampler;
};

//CALL IN PIXEL SHADER
GBufferOut opaqueShader(in float3 pos, in row_major float3x3 TBN, in float2 UV, in OpaqueMaterial material, in OpaqueTexSamplers samplers)
{
	float3 albedo = material.albedo.Sample(samplers.generalSampler, UV).rgb;

	bool hasNormals   = (material.flags >> 0) & 1;
	bool hasRoughness = (material.flags >> 1) & 1;
	bool hasMetalness = (material.flags >> 2) & 1;
	bool invertY      = (material.flags >> 3) & 1;
	bool buildBlue    = (material.flags >> 4) & 1;
	float metalness, roughnessLinear;
	float3 microNormal;

	if (hasNormals)
	{
		microNormal = unpackNormalMap(material.normal.Sample(samplers.generalSampler, UV), invertY, buildBlue);
	}
	else
	{
		microNormal = material.auxNormal;
	}
	microNormal = normalize(mul(microNormal, TBN));

	roughnessLinear = material.auxRoughness;

	if (hasRoughness)
	{
		roughnessLinear = material.rough.Sample(samplers.generalSampler, UV);
	}

	metalness = material.auxMetalness;
	if (hasMetalness)
	{
		metalness = material.metal.Sample(samplers.generalSampler, UV);
	}

	float2 microNormalPacked = packOctahedron(microNormal);
	float2 macroNormalPacked = packOctahedron(TBN[2]);

	float3 emission = float3(0.0f, 0.0f, 0.0f);

	GBufferOut gbOut = { albedo, float4(microNormalPacked, macroNormalPacked), float2(roughnessLinear, metalness), emission };
	return gbOut;
}
#endif //_OPAQUE_SHADER_HLSLI_