#include "../cbPerView.hlsli"
#include "../Common.hlsli"
struct VSIn
{
	float3 lPos : POSITION;
	float3 lNormal : NORMAL;
	float3 lTangent: TANGENT;
	float3 lBitangent : BITANGENT;
	float2 uv      : UV;

	float4 MWRight : MW0;
	float4 MWUp    : MW1;
	float4 MWFwd   : MW2;
	float4 MWPos   : MW3;
	uint   objectID : ID;
};

struct VSOut
{
	float4 clip        : SV_POSITION;
	float3 pos         : POSITION;
	float3 normal : NORMAL;
	float3 tangent     : TANGENT;
	float3 bitangent   : BITANGENT;
	float2 uv          : UV;
	nointerpolation uint objectID : ID;
};

cbuffer PerDraw : register(cb_PER_DRAW)
{
	row_major float4x4 g_meshModel;
}

VSOut main(VSIn vin) 
{
	VSOut vout;

	row_major float4x4 modelToWorld = float4x4(vin.MWRight, vin.MWUp, vin.MWFwd, vin.MWPos);
	float4x4 meshToModel = float4x4(g_meshModel);
	float4x4 worldToClip = float4x4(g_VP);

	float3 worldPos = mul(mul(float4(vin.lPos, 1.0f), g_meshModel), modelToWorld).xyz;

	float3 normalModel = mul(float4(vin.lNormal.xyz, 0.0f), meshToModel).xyz;
	float3 tangentModel = mul(float4(vin.lTangent.xyz, 0.0f), meshToModel).xyz;
	float3 bitangentModel = mul(float4(vin.lBitangent.xyz, 0.0f), meshToModel).xyz;


	
//Just pick a space to calculate everything(?). That would also mean cleaner view uniforms
#ifdef ILLUMINATION_VIEW_SPACE

	vout.clip      = mul(float4(worldPos, 1.0f), g_VP);
	vout.pos       = mul(float4(worldPos, 1.0f), g_V).xyz;
	vout.normal    = transformVectorOrthonormal(transformVector(normalModel, modelToWorld)      , g_V);
	vout.tangent   = transformVectorOrthonormal(transformVector(tangentModel, modelToWorld)     , g_V);
	vout.bitangent = transformVectorOrthonormal(transformVector(bitangentModel, modelToWorld)   , g_V);

#elif CAMERA_CENTER_WS

	row_major float3x3 cameraRotation = float3x3(g_V[0].xyz, g_V[1].xyz, g_V[2].xyz);

	vout.clip      = mul(float4(mul(worldPos, cameraRotation), 1.0f), g_P);
	vout.pos       = worldPos.xyz;
	vout.normal    = transformVector(normalModel, modelToWorld);
	vout.tangent   = transformVector(tangentModel, modelToWorld);
	vout.bitangent = transformVector(bitangentModel, modelToWorld);

#else

	vout.clip      = mul(float4(worldPos, 1.0f), g_VP);
	vout.pos       = worldPos.xyz;
	vout.normal    = transformVector(normalModel, modelToWorld);
	vout.tangent   = transformVector(tangentModel, modelToWorld);
	vout.bitangent = transformVector(bitangentModel, modelToWorld);

#endif

	vout.uv = vin.uv;
	vout.objectID = vin.objectID;
	return vout;
}