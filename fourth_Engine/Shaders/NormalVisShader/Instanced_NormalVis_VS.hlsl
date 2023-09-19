#include "../cbPerView.hlsli"
struct VSIn
{
	float3 posL:           POSITION;
	float3 normalL:        NORMAL;
	//float3 tangentL:       TANGENT;
	//float3 bitangentL:     BITANGENT;

	//Instance model data
	float4 MWRight:             MW0;
	float4 MWUp:                MW1;
	float4 MWForward:           MW2;
	float4 MWPos:               MW3;


};

struct VSOut
{
	float4 posClip: SV_POSITION;
	float3 posWorld : WPOSITION;
	float3 normalWorld: WNORMAL;
};

cbuffer cbPerDraw : register(cb_PER_DRAW)
{
	row_major float4x4 g_MM;
}


VSOut main(VSIn vin)
{

	VSOut vout;

	float4 pos = float4(vin.posL.xyz, 1.0f);

	float4x4 meshToModel = float4x4(g_MM);
	row_major float4x4 modelToWorld = float4x4(vin.MWRight, vin.MWUp, vin.MWForward, vin.MWPos);
	float4x4 worldToClip = float4x4(g_VP);

	float3 axisX = normalize(modelToWorld[0].xyz);
	float3 axisY = normalize(modelToWorld[1].xyz);
	float3 axisZ = normalize(modelToWorld[2].xyz);
	float3 normal = mul(float4(vin.normalL.xyz, 0.0f), meshToModel).xyz;

	float3 posWorld = mul(mul(pos, meshToModel), modelToWorld).xyz;
	vout.posClip = mul(float4(posWorld, 1.0f), worldToClip);
	vout.posWorld = posWorld;
	vout.normalWorld = normal.x * axisX + normal.y * axisY + normal.z * axisZ;


	return vout;
}