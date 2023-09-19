#include "../Registers.hlsli"
#include "../sSamplers.hlsli"
#include "IBLOperations.hlsli"

struct GSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
    uint slice : SV_RenderTargetArrayIndex;
};

cbuffer SamplingProperties : register(cb_IBL_GEN)
{
	uint g_numSamples;
	float g_roughness2;
}

TextureCube g_cubemap : register(srv_ALBEDO);

float4 main(GSOutput gsOut) : SV_TARGET
{
	//to test that sampling cube texel from UV and slice is correct
    //float3 color = g_cubemap.Sample(g_pointWrap, cubeDirectionFromUV(gsOut.uv, gsOut.slice)).rgb;
    
    uint texWidth;
    uint texHeight;
    uint levels;
    g_cubemap.GetDimensions(0, texWidth, texHeight, levels);
    
    //TODO: Just upload this in register
    uint texRes = max(texWidth, texHeight);
    
    float3 N = lookupCubemapFrom_UVFace(gsOut.uv, gsOut.slice);
    
    float3 integral = computeIBLSpecularIrradiance(N, g_numSamples, g_roughness2 * g_roughness2, texRes, g_cubemap);

	return float4(integral, 1.0f);
}