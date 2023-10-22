#include "../Common.hlsli"

#ifndef _APERTURE_DISPERSION_CS_HLSL
#define _APERTURE_DISPERSION_CS_HLSL

Texture2D<float>           sourcePattern : register(SRV_PP_0);
RWTexture2D<float3> polychromaticPattern : register(UAV_PP_COMPUTE_0);


cbuffer dispersionParams : register(CB_PP_0)
{
    uint  g_numSamples;
    uint  g_patternSize;
    float pad;
    float pad2;
};

#define TILE_SIZE 8
[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    const uint2 texel = DTid.xy;
    const float invSize = 1.0f / float(g_patternSize);
    const float halfTexel = invSize;

    const float2 currentUV = (float2(texel) + float2(halfTexel, halfTexel)) * invSize;
    
    for (uint s = 0; s < g_numSamples; ++s)
    {
        
    }

}


#endif