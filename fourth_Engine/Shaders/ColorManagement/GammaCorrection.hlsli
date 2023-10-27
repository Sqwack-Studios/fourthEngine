#ifndef _GAMMA_CORRECTION_HLSLI
#define _GAMMA_CORRECTION_HLSLI

//This constant came from Epic "GammaCorrectionCommon.ush"
#define MIN_NON_DENORMAL 6.10352e-5f

#define LIN_2_sRGB_TRANSFER_LIMIT 0.0031308f
#define sRGB_2_LIN_TRANSFER_LIMIT 0.04045f


float3 linearRGB_to_sRGB(in float3 RGB, in const float gamma = 2.4f)
{
    RGB = max(MIN_NON_DENORMAL, RGB);

    return min(RGB * 12.92f, pow(max(RGB, LIN_2_sRGB_TRANSFER_LIMIT), 1.0f / gamma) * 1.055f - 0.055f);
}

float3 sRGB_to_linearRGB(in float3 sRGB, in const float gamma = 2.4f)
{
    sRGB = max(MIN_NON_DENORMAL, sRGB);
    
    return min(sRGB * 0.07739f, pow(max(sRGB, sRGB_2_LIN_TRANSFER_LIMIT) * 0.947867f + 0.052132f, gamma));

}

float3 linearRGB_to_sRGB_fast(in float3 RGB, in const float gamma = 2.2f)
{
    return pow(RGB, 1.0f / gamma);
}

float3 sRGB_to_linear_fast(in float3 sRGB, in const float gamma = 2.2f)
{
    return pow(sRGB, gamma);
}

#endif //_GAMMA_CORRECTION_HLSLI