#include "../cbPerView.hlsli"

struct VSIn
{
	float3 lPos : POSITION;

	//instance
	float4 MWRight : MW0;
	float4 MWUp    : MW1;
	float4 MWFwd   : MW2;
	float4 MWPos   : MW3;
};

cbuffer PerDraw : register(cb_PER_DRAW)
{
	row_major float4x4 g_meshModel;
}

float4 main(VSIn vin) : SV_POSITION
{

	row_major float4x4 modelToWorld = float4x4(vin.MWRight, vin.MWUp, vin.MWFwd, vin.MWPos);
	float4x4 meshToModel = float4x4(g_meshModel);
	float4x4 worldToClip = float4x4(g_VP);

	float3 worldPos = mul(mul(float4(vin.lPos, 1.0f), g_meshModel), modelToWorld).xyz;


	float4 clip = mul(float4(worldPos, 1.0f), g_VP);


	return clip;
}