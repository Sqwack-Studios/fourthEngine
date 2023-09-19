#include "../cbPerView.hlsli"

struct VSIn
{
	float3 position : POSITION;
	//instance
	float4 DWRight  : DW0;
	float4 DWUp     : DW1;
	float4 DWFwd    : DW2;
	float4 DWPos    : DW3;

	float4 WDRight  : WD0;
	float4 WDUp     : WD1;
	float4 WDFwd    : WD2;
	float4 WDPos    : WD3;
	float4 color    : COLOR;
	uint   pID      : PID;
};

struct VSOut
{
	float4 clip                                   : SV_POSITION;
	nointerpolation row_major float4x4 worldDecal : WORLD_DECAL;
	nointerpolation float4 color                  : COLOR;
	nointerpolation uint   parentObjID            : PID;
};

VSOut main(VSIn vin)
{
	row_major float4x4 worldDecal = float4x4(vin.WDRight, vin.WDUp, vin.WDFwd, vin.WDPos);
	row_major float4x4 decalWorld = float4x4(vin.DWRight, vin.DWUp, vin.DWFwd, vin.DWPos);

	float3 worldPos = mul(float4(vin.position, 1.0f), decalWorld).xyz;
	float4 clip;

#ifdef CAMERA_CENTER_WS
	//we just want rotation, not translation
	row_major float3x3 cameraRot = float3x3(float3(g_V[0].xyz), float3(g_V[1].xyz), float3(g_V[2].xyz));
	clip = mul(float4(mul(worldPos, cameraRot), 1.0f), g_P);

#else

	clip = mul(float4(worldPos, 1.0f), g_VP);

#endif

	VSOut vout = {clip, worldDecal, vin.color, vin.pID};

	return vout;
}