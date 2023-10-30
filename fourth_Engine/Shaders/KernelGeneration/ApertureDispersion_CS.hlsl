#include "../Common.hlsli"
#include "../ColorManagement/CIEColorMatching.hlsli"
#include "../ColorManagement/ColorSpace.hlsli"
#include "../ColorManagement/GammaCorrection.hlsli"
#include "../cbPerFrame.hlsli"

#include "../PostProcessing/Resolve.hlsli"
#ifndef _APERTURE_DISPERSION_CS_HLSL
#define _APERTURE_DISPERSION_CS_HLSL

#define TILE_SIZE 8

Texture2D<float>           sourcePattern : register(SRV_PP_0);
RWTexture2D<float4> polychromaticPattern : register(UAV_PP_COMPUTE_0);
StructuredBuffer<float>    invDCvalue    : register(SRV_PP_1);

static const float sampleWave[3] = { 0.630f, 0.530f, 0.465f }; //RGB wavelengths


//This shader performs polychromatic dispersion to the Fourier Transform of an aperture under the assumptions of Far Field Fraunhofer diffraction
// source : https://onlinelibrary.wiley.com/doi/10.1111/j.1467-8659.2009.01357.x
//
// This shader will do as many lookups as number of samples are required from the visible spectrum, and it assumes a squared input. For each wavelength, a spatial domain coordinate
// will be deduced based on physical properties of a sensor/human eye and sampled wavelength, effectively scaling the aperture for each wavelength.
//
// For each wavelength, we will also assume that the aperture was lit by a standard Illuminant D65, so we will also sample a relative SPD_65, transform 
// into XYZ coordinates and then into RGB. Finally, a energy conservation normalization could be employed.
// |----------------------------------------------------------Inputs---------------------------------------------------------------
// |NOTE: spatial variables are expected in micrometers.
// |
// |     ------>Texture2D<float> sourcePattern: it's the output of the Fourier Transform. The scale is important, as we have to make a lot of operations over float types.
// |           Applying arithmetic operations to very different floats in size can rapidly cause precision issues
// |
// |     ------>RWTexture2D<float3> polychromaticPattern: polychromatic pattern affected by dispersion, assuming Fraunhofer diffraction.
// |
// |    ************************************************  Uniforms  ************************************************
// |
// |    -------> uint     g_numSamples      : num of visible spectrum wavelengths to sample. This is the number of texture lookups per destination texel.
// |    -------> float2   g_signalLength    : signalLength.x, invSingalLength.y
// |    -------> float1   g_maxFrequency    : maximum spatial frequency representable by the physical shape of the signal receiver (sensor/human eye). It also depends of the resolution of the FFT
// |                                        maxFreq = length/physicalSize * 0.5
// |    -------> float1   g_frequencyStep   : size of the FFT bins. Each texel evaluated at the center is a step of 1/physicalSize
// |    -------> float1   g_focalDistance   : distance between the sensore receiver and the plane/mask/aperture responsible for the diffraction. It is needed to evaluate the spatial domain of 
// |                                        the diffraction pattern. The relationship is based on fraunhofer diffraction: freq = x / (wavelength * focalDistance)
// |    -------> float1   g_spatialStep     : physical size of each sensor pixel
// |    -------> float2   g_kernelCenter    : UV coordinates that define where is the center of the kernel. This is used to match frequency/space coordinates and find the center
// | //TODO: Add fresnel term. Deduce how to maybe speedup things using LDS, because we will be probably sampling the same texel multiple times.
// |
// |
// |
// |------------------------------------------------------------------------------------------------------------------------------------


cbuffer dispersionParams : register(CB_PP_0)
{
    uint     g_numSamples;
    float2   g_signalLength;
    float    g_maxFrequency;
    //
    float    g_frequencyStep;
    float    g_focalDistance;
    float    g_spatialStep;
    float    g_wavelength;
    //
    float2   g_kernelCenter;
    float    pad1[2];
};

[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    const uint2 targetTexel = DTid.xy; //This is the texel were we want to compute the dispersion
    const float invDC = invDCvalue[0];
    if (targetTexel.x >= uint(g_signalLength.x) || targetTexel.y >= uint(g_signalLength.x))
        return;

   const uint signalLength = float(g_signalLength.x);
   const float invSignalLength = g_signalLength.y;
   
   //We have to compute the position of this texel in the sensor. Then, for each wavelength, we compute which frequency is that, and sample.
   const float2 position = (float2(targetTexel) - g_kernelCenter * float(signalLength)) * g_spatialStep;
   
   float3 finalColor = float3(0.0f, 0.0f, 0.0f);
   
   uint idx = 0;
   for (float wave = 380.0f; wave < 830.0f; wave += 1.0f)
   {
        const float wavelength_um = wave * 1e-3f;
        const float wavelength_nm = wave;
        const float2 frequencyUV = (position / (g_focalDistance * wavelength_um))/g_maxFrequency + g_kernelCenter; //fourier domain coords displaced by the kernel center
        const float scale = 10000.0f / (wavelength_um * wavelength_um * g_focalDistance * g_focalDistance);
        //now we sample D65 illuminant for this wavelength
        const float D65_spectrum = D65_DATASET2[idx];
        float3 XYZ = xyz_CIE1931_COLOR_MATCHING[idx + 20];
        XYZ *= D65_spectrum * 0.003975f * LUMINOUS_EFFICACY;
        
        XYZ = clipRGB_add_white(XYZ_to_RGB(XYZ));
        //XYZ *= 1.85f;
        //
        //float a = max(XYZ.x, max(XYZ.y, XYZ.z)) + 1e-4f;
        //XYZ /= a;
        finalColor +=  scale * XYZ * sourcePattern.SampleLevel(g_trilinearClamp, frequencyUV, 0.0f).r;
        ++idx;
   }
   
    
   //for (uint s = 0; s < 3; ++s)
   //{
   //    const float wavelength_um = sampleWave[s];
   //    const float wavelength_nm = wavelength_um * 1e+3;
   //    const float2 frequencyUV = (position / (g_focalDistance * wavelength_um))/g_maxFrequency + g_kernelCenter; //fourier domain coords displaced by the kernel center
   //    const float scale = 10000.0f / (wavelength_um * wavelength_um * g_focalDistance * g_focalDistance);
   //    
   //    float3 XYZ = float3(x_Fit_1931(wavelength_nm), y_Fit_1931(wavelength_nm), z_Fit_1931(wavelength_nm));
   //    
   //    finalColor += scale * clipRGB_add_white(XYZ_to_RGB(XYZ)) * invDC * sourcePattern.SampleLevel(g_bilinearClamp, frequencyUV, 0.0f).r;
   //}
    
    finalColor = adjustExposure(finalColor, g_EV100);
    finalColor = clipRGB(linearRGB_to_sRGB(acesHdr2Ldr(finalColor)));
   polychromaticPattern[targetTexel] = float4(finalColor, 1.0f);

  //float3 finalColor = clipRGB(XYZ_to_RGB(xyz_CIE1931_COLOR_MATCHING[targetTexel.x % 471]));
  //
  //polychromaticPattern[targetTexel] = float4(linearRGB_to_sRGB(finalColor), 1.0f);
}


#endif