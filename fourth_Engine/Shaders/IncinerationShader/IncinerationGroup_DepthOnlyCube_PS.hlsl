#include "IncinerationEffect.hlsli"



Texture2D<float1> g_noise : register(srv_INC_NOISE);


struct GSOut
{
	float4 clip : SV_POSITION;
	float2 uv  : UV;
	float distance : DISTANCE;
	nointerpolation float radius : RADIUS;
	nointerpolation float prevRadius : PRADIUS;
	uint   targetSlice : SV_RenderTargetArrayIndex;
};

void main(GSOut gout) 
{
	IESampleNoise sampling = { g_noise, g_trilinearWrap };
	float vis = IE_DiscardSurface(gout.distance, gout.radius, gout.prevRadius, gout.uv, sampling);
}