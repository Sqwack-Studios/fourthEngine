#include "../Registers.hlsli"
//DISSOLUTION EFFECT
Texture2D<float1> g_dissolutionNoise : register(srv_DIS_NOISE);

#include "DissolutionEffect.hlsli"

struct VSOut
{
	float4 worldPos : SV_POSITION;
	float2 uv       : UV;
	float2 time     : TIME;
};

void main(VSOut vout)
{
	float sampledNoise = sampleNoise(vout.uv, vout.time.x, vout.time.y);
	bool shouldDiscard = vout.time.y <  sampledNoise;

    if (shouldDiscard)
    {
    	discard;
    }
}