#include "IncinerationEffect.hlsli"

struct VSOut
{
	float4 clip : SV_POSITION;
	float2 uv : UV;
	float distance : DISTANCE;
	nointerpolation float radius : RADIUS;
	nointerpolation float prevRadius : PREVRADIUS;
};

Texture2D<float1> g_noise : register(srv_INC_NOISE);

void main(VSOut vout) 
{
	IESampleNoise sampling = { g_noise, g_trilinearWrap };
	float vis = IE_DiscardSurface(vout.distance, vout.radius, vout.prevRadius, vout.uv, sampling);
}