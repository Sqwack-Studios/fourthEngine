struct GSInput
{
	float3 color : COLOR;
	float globalScale : SCALE;
	float3 posLocal : LPOSITION;
	float3 posWorld : WPOSITION;
	float3 normalWorld : WNORMAL;
};

#include "../NormalDebugTriangle.hlsli"

