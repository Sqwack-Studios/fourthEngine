#include "../Registers.hlsli"
#include "../cbPerView.hlsli"

struct VSIn
{
	float3 lPos : POSITION;
	float2 uv   : UV;

	//instance
	float4 MWRight : MW0;
	float4 MWUp    : MW1;
	float4 MWFwd   : MW2;
	float4 MWPos   : MW3;
	float2 time    : TIME;

};

cbuffer PerDraw : register(cb_PER_DRAW)
{
	row_major float4x4 g_meshModel;
};

struct VSOut
{
	float4 clip     : SV_POSITION;
	float2 uv       : UV;
	float2 time     : TIME;
};

VSOut main(VSIn vin)
{
	VSOut vout;

	row_major float4x4 modelToWorld = float4x4(vin.MWRight, vin.MWUp, vin.MWFwd, vin.MWPos);
	float4x4 meshToModel = float4x4(g_meshModel);

	float3 worldPos = mul(mul(float4(vin.lPos, 1.0f), g_meshModel), modelToWorld).xyz;

	vout.clip = mul(float4(worldPos, 1.0f), g_VP);
	vout.uv = vin.uv;
	vout.time = vin.time;

	return vout;
}