#include "DxyIlluminant_helpers.hlsli"

#ifndef _CIE_COLOR_MATCHING_HLSLI
#define _CIE_COLOR_MATCHING_HLSLI

//NVidia: Simple Analytic Approximations to the CIE XYZ Color Matching Functions
//https://research.nvidia.com/publication/2013-07_simple-analytic-approximations-cie-xyz-color-matching-functions 
//https://jcgt.org/published/0002/02/01/paper.pdf

float x_Fit_1931(in const float wavelength) //in nanometers
{
     float t1 = (wavelength - 442.0f) * ((wavelength < 442.0f) ?    0.0624f : 0.0374f);
     float t2 = (wavelength - 599.8f) * ((wavelength < 599.8f) ?    0.0264f : 0.0323f);
     float t3 = (wavelength - 501.1f) * ((wavelength < 501.1f) ?    0.0490f : 0.0382f);

     return 0.362f * exp(-0.5f * t1 * t1) + 1.056f * exp(-0.5f * t2 * t2) - 0.065f * exp(-0.5f * t3 * t3);
     
}

float y_Fit_1931(in const float wavelength) //in nanometers
{
     float t1 = (wavelength - 568.8f) * ((wavelength < 568.8f) ?    0.0213f : 0.0247f);
     float t2 = (wavelength - 530.9f) * ((wavelength < 530.9f) ?    0.0613f : 0.0322f);

     return 0.362f * exp(-0.5f * t1 * t1) + 1.056f * exp(-0.5f * t2 * t2);
     
}

float z_Fit_1931(in const float wavelength) //in nanometers
{
     float t1 = (wavelength - 437.0f) * ((wavelength < 437.0f) ?    0.0845f : 0.0278f);
     float t2 = (wavelength - 459.0f) * ((wavelength < 459.0f) ?    0.0385f : 0.0725f);

     return 0.362f * exp(-0.5f * t1 * t1) + 1.056f * exp(-0.5f * t2 * t2);
     
}




//CIE D65 transformations
float sample_Dxy_illuminant(in const float wavelength, in const float2 xyCoords)
{
    const uint idx = getIlluminantD_wavelengthIndex(wavelength);
    
    float M1, M2;
    compute_D_Illuminant_M1M2(xyCoords, M1, M2);
    
    float3 S012 = D_S012[idx];
    
    return spectrum_Dxy_Illuminant(M1, M2, S012.x, S012.y, S012.z);
}


//Performs a discrete integration of a Dxy spectrum between [min, max] wavelengths. 5 nm per step.
float sample_Dxy_illuminant(in const float minWavelength, in const float maxWavelength, in const float2 xy_white)
{
    float integration = 0.0f;
    
    
    
    return integration;
}

#endif //_CIE_COLOR_MATCHING_HLSLI