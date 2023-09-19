#include "../Registers.hlsli"

Texture2DMS<float> g_depthBuffer : register(srv_RESOLVE_DEPTH);

#include "ResolveDepthTextureMS.hlsli"

struct VSOut
{
	float4 pos : SV_POSITION;
};

float main(VSOut vout) : SV_TARGET
{
	return resolveDepth(vout.pos.x, vout.pos.y);
}