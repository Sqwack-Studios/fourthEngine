#include "cbPerView.hlsli"

struct VSIn
{
	float3 posL:           POSITION;
	float3 normalL:        NORMAL;
	float3 tangentL:       TANGENT;
	float3 bitangentL:     BITANGENT;
	//Instance model data
	float4 MWRight:        MW0;
	float4 MWUp:           MW1;
	float4 MWForward:      MW2;
	float4 MWPos:          MW3;
	//Instance attribute
	float4 color:        COLOR;
};

struct VSOut
{
	float4 posClip : SV_POSITION;
	float4 color : COLOR;
	float3 posWorld : POSITION;
	float3 normalWorld : NORMAL;

};



//mesh to model matrices
cbuffer cbPerDraw : register(b2)
{
	row_major float4x4 g_MM;

}

VSOut main(VSIn vin) 
{
	VSOut vout;

	float4 pos = float4(vin.posL.xyz, 1.0f);

	float4x4 meshToModel = float4x4(g_MM);
	float4x4 worldToClip = float4x4(g_VP);
	row_major float4x4 modelToWorld = float4x4(vin.MWRight, vin.MWUp, vin.MWForward, vin.MWPos);

	vout.posWorld = mul(mul(pos, meshToModel), modelToWorld).xyz;
	vout.posClip = mul(float4(vout.posWorld, 1.0f), worldToClip);
	vout.normalWorld = mul(mul(float4(vin.normalL.xyz, 0.0f), meshToModel), modelToWorld).xyz;
	vout.color = vin.color;


	return vout;
}