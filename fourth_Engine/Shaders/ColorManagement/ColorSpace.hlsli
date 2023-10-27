#ifndef _COLOR_SPACE_HLSLI
#define _COLOR_SPACE_HLSLI

//http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

//---------------------------------------------------
//
//     Conversion matrices between different color spaces.
//     These matrices assume that XYZ are in nominal range and a standard D65 white point illuminant (T = 6504K)
//    
//     For a specialized conversion matrix between a non-standard white point, consult:
//
//                   http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
//
//     A chromatic adaptation algorith may also be adopted to convert between different reference whites
//                  
//                   http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
//
//--------------------------------------------------
static const float3x3 XYZ_TO_LINEAR_RGB =
{
    + 3.2404542f, - 1.5371385f, - 0.4985314f,
    - 0.9692660f, + 1.8760108f, + 0.0415560f,
    + 0.0556434f, - 0.2040259f, + 1.0572252f
};


static const float3x3 LINEAR_RGB_TO_XYZ =
{
    + 0.4124564f, + 0.3575761f, + 0.1804375f,
    + 0.2126729f, + 0.7151522f, + 0.0721750f,
    + 0.0193339f, + 0.1191920f, + 0.9503041f
};                


//Converts XYZ tristimulus values to RGB linear (not sRGB)
float3 XYZ_to_RGB(in const float3 XYZ)
{
    return mul(XYZ_TO_LINEAR_RGB, XYZ);
}

//Converts linear RGB to XYZ
float3 RGB_to_XYZ(in const float3 RGB)
{
    return mul(LINEAR_RGB_TO_XYZ, RGB);
}

float3 clipRGB(in const float3 RGB)
{
    return clamp(RGB, 0.0f, 1.0f);
}
#endif //_COLOR_SPACE_HLSLI
