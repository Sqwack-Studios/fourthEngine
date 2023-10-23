#include "../Common.hlsli"

#ifndef _APERTURE_DISPERSION_CS_HLSL
#define _APERTURE_DISPERSION_CS_HLSL

Texture2D<float>           sourcePattern : register(SRV_PP_0);
RWTexture2D<float3> polychromaticPattern : register(UAV_PP_COMPUTE_0);

//This shader performs polychromatic dispersion to the Fourier Transform of an aperture under the assumptions of Far Field Fraunhofer diffraction
// source : https://onlinelibrary.wiley.com/doi/10.1111/j.1467-8659.2009.01357.x
//
// This shader will do as many lookups as number of samples are required from the visible spectrum, and it assumes a squared input. For each wavelength, a spatial domain coordinate
// will be deduced based on physical properties of a sensor/human eye and sampled wavelength, effectively scaling the aperture for each wavelength.
//
// For each wavelength, we will also assume that the aperture was lit by a standard Illuminant D65, so we will also sample a relative SPD_65, transform 
// into XYZ coordinates and then into RGB. Finally, a energy conservation normalization could be employed.
//-----------------------------------------------------------Inputs---------------------------------------------------------------
//
//
//      ------>Texture2D<float> sourcePattern: it's the output of the Fourier Transform. The scale is important, as we have to make a lot of operations over float types.
//            Applying arithmetic operations to very different floats in size can rapidly cause precision issues
//
//      ------>RWTexture2D<float3> polychromaticPattern: polychromatic pattern affected by dispersion, assuming Fraunhofer diffraction.
//
//     -----------------------------------------Uniforms------------------------------
//
//     -------> uint     g_numSamples      : num of visible spectrum wavelengths to sample. This is the number of texture lookups per destination texel.
//     -------> float2    g_signalLength   : signalLength.x, invSingalLength.y
//     -------> float1   g_maxFrequency    : maximum spatial frequency representable by the physical shape of the signal receiver (sensor/human eye). It also depends of the resolution of the FFT
//                                         maxFreq = length/physicalSize * 0.5
//     -------> float1   g_frequencyStep   : size of the FFT bins. Each texel evaluated at the center is a step of 1/physicalSize
//     -------> float1   g_focalDistance   : distance between the sensore receiver and the plane/mask/aperture responsible for the diffraction. It is needed to evaluate the spatial domain of 
//                                         the diffraction pattern. The relationship is based on fraunhofer diffraction: freq = x / (wavelength * focalDistance)
//     
//
//  //TODO: Add fresnel term. Deduce how to maybe speedup things using LDS, because we will be probably sampling the same texel multiple times.
//
cbuffer dispersionParams : register(CB_PP_0)
{
    uint     g_numSamples;
    float2   g_signalLength;
    float    g_maxFrequency;
    float    g_frequencyStep;
    float    g_focalDistance;
};

#define TILE_SIZE 8
[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    const uint2 texel = DTid.xy; //This is the texel were we want to compute the dispersion

    const uint signalLength = float(g_signalLength.x);
    const float invSignalLength = g_signalLength.y;
    
    for (uint s = 0; s < g_numSamples; ++s)
    {
        
    }

}


#endif