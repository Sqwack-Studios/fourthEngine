#include "../Registers.hlsli"
//DISSOLUTION EFFECT
Texture2D<float1> g_dissolutionNoise : register(srv_DIS_NOISE);

#include "DissolutionEffect.hlsli"


struct GSOutput
{
	float4 pos : SV_POSITION;
	uint   targetSlice : SV_RenderTargetArrayIndex;
	float2 uv       : UV;
	float2 time     : TIME;
};

void main(GSOutput gsout)
{
	float sampledNoise = sampleNoise(gsout.uv, gsout.time.x, gsout.time.y);
	bool shouldDiscard = gsout.time.y < sampledNoise;

	if (shouldDiscard)
	{
		discard;
	}
}