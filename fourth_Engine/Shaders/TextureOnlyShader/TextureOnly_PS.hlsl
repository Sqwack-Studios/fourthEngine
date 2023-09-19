#include "../sSamplers.hlsli"
#include "../Common.hlsli"
#include "../RenderTargets.hlsli"
Texture2D<float4>   g_albedo : register(srv_ALBEDO);

struct PSIn
{
	float4 posScreen : SV_POSITION;
	float3 posWorld    : WPOSITION;
	float3 normalWorld : WNORMAL;
	float2 uv : UV;
};

float4 main(PSIn pin) : TARGET_EMISSION
{
	float3 albedo = g_albedo.Sample(g_pointWrap, pin.uv.xy).xyz;

	return float4(albedo, 1.0f);
}