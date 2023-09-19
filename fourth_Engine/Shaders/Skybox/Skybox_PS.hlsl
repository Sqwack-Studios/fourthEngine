#include "../sSamplers.hlsli"


TextureCube g_cubemap : register(srv_SKYMAP);

struct PSIn
{
	float4 screenPos : SV_POSITION;
	float3 direction : DIRECTION;
};

float4 main(PSIn pin) : SV_TARGET
{
	return g_cubemap.Sample(g_trilinearWrap, pin.direction);
	//return float4(0.0f, 0.0f, 0.0f, 1.0f);
}