struct GSInput
{
	float4 posClip        : SV_POSITION;
	float3 posWorld       : WPOSITION;
	float3 normalWorld    : WNORMAL;
	nointerpolation float3 emissionColor     : COLOR;
};

#include "../NormalDebugTriangle.hlsli"

