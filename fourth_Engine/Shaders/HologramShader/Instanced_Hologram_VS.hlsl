#include "../cbPerView.hlsli"
#include "../Common.hlsli"
//Because VS is invoked per vertex, and GS is invoked per primitive, I'm moving this code
//to GS. However, if we add tesselation, VS invocations will remain constant and GS invocations could implode
//At the end, there's a tradeoff and generating new triangles is not free
//Ask Emil?


struct VSIn
{
	float3 posL:           POSITION;
	float3 normalL:        NORMAL;
	//float3 tangentL:       TANGENT;
	//float3 bitangentL:     BITANGENT;
	//Instance model data
	float4 MWRight:        MW0;
	float4 MWUp:           MW1;
	float4 MWForward:      MW2;
	float4 MWPos:          MW3;
	//Instance attribute
	float3 color:        COLOR;
};

struct VSOut
{
	float3 color: COLOR;
	float  globalScale : SCALE;
	float3 posLocal: LPOSITION;
	float3 posWorld: WPOSITION;
	float3 normalWorld: WNORMAL;
};

cbuffer cbPerDraw : register(cb_PER_DRAW)
{
	row_major float4x4 g_MM;

}


VSOut main( VSIn vin )
{
	VSOut vout;

	float4x4 meshToModel = float4x4(g_MM);
	row_major float4x4 modelToWorld = float4x4(vin.MWRight, vin.MWUp, vin.MWForward, vin.MWPos);

	float3 posLocal = mul(float4(vin.posL.xyz, 1.0f), meshToModel).xyz;
	float3 normalLocal = mul(float4(vin.normalL.xyz, 0.0f), meshToModel).xyz;
	float3 posWorld = mul(float4(posLocal, 1.0f), modelToWorld).xyz;

	float invxScale = 1.0f / length(modelToWorld[0].xyz);
	float invyScale = 1.0f / length(modelToWorld[1].xyz);
	float invzScale = 1.0f / length(modelToWorld[2].xyz);


	vout.color = vin.color;
	//Make a weighted average, where the smallest scale is the heavier?Right now, we could create a 10, 5, 1 scale and distortion would be huge
	vout.globalScale = (1.0f / invxScale + 1.0f / invyScale + 1.0f / invzScale) * 0.3333f;
	vout.posLocal = posLocal;
	vout.posWorld = posWorld;
	vout.normalWorld = transformVectorOrthonormal(normalLocal, modelToWorld);


	return vout;
}