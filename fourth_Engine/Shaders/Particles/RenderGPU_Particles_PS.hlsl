#include "../sSamplers.hlsli"
Texture2D<float1>  g_mask : register(srv_PARTICLE_MASK);

struct VSOut
{
	float4 clip : SV_POSITION;
	float2 uv : TEXCOORDS;
	nointerpolation float4 emission : EMISSION;
};


float4 main(VSOut vout) : SV_TARGET
{
	float1 mask = g_mask.Sample(g_pointWrap, vout.uv);


	float4 outColor = vout.emission * mask;

	float2 translateUV = vout.uv - 0.5f;
	float decay = (1.0f - length(translateUV));
	outColor.rgb *= decay * decay;

	return outColor;
}