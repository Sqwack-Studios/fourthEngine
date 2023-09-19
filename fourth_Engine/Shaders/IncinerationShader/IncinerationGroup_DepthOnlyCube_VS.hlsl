#include "../cbPerView.hlsli"
struct VSIn
{
	float3 lPos : POSITION;
	float2 uv : UV;
	//
	float4 MWRight : MW0;
	float4 MWUp    : MW1;
	float4 MWFwd   : MW2;
	float4 MWPos   : MW3;
	float3 spherePos : SPHEREPOS;
	float2 sphereRadius : SPHRADIUS;
};

struct VSOut
{
	float3 pos : POSITION;
	float2 uv  : UV;
	float distance : DISTANCE;
	nointerpolation float radius : RADIUS;
	nointerpolation float prevRadius : PRADIUS;
};

cbuffer PerMesh : register(cb_PER_DRAW)
{
	row_major float4x4 g_meshToModel;
	float              g_maxRadius;
	float              pad[3];
}
VSOut main( VSIn vin)
{
	row_major float4x4 modelToWorld = float4x4(vin.MWRight, vin.MWUp, vin.MWFwd, vin.MWPos);
	float4x4 meshToModel = float4x4(g_meshToModel);

	float3 modelPos = mul(float4(vin.lPos, 1.0f), meshToModel).xyz;
	float3 worldPos = mul(float4(modelPos, 1.0f), modelToWorld).xyz;

	VSOut vout;
	vout.pos = worldPos;
	vout.uv = vin.uv;
	vout.distance = length(vin.spherePos - modelPos);
	vout.radius = vin.sphereRadius.x;
	vout.prevRadius = vin.sphereRadius.y;

	return vout;
}