#ifndef _COLOR_SPACE_HLSLI
#define _COLOR_SPACE_HLSLI

//http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
//https://aty.sdsu.edu/explain/optics/rendering.html
//https://scipython.com/blog/converting-a-spectrum-to-a-colour/
//https://www.fourmilab.ch/documents/specrend/
//
//From "Billmeyer and Saltzman's principles of color technology", 4th Edition, regarding to XYZ tristimulus integration, page 67:
//           
//
//          There are many colors that are not materials, such as lights and displays. 
//          When calculating their tristimulus values, the spectral-reflectance factor is not included in the tristimulus-integration equations.
//          Furthermore, the convention of normalizing Y such that it equals 100 or 1 is generally not used.
//          Instead, photometric units are used, for which the normalizing constant in the tristimulus equations, k, equals 683 lm/W, known as the maximum luminous efficacy
//           
//           X = K_m · integral{ phi(lambda) · CMF_x(lambda) deltaLambda }
//           Y = K_m · integral{ phi(lambda) · CMF_y(lambda) deltaLambda }
//           Z = K_m · integral{ phi(lambda) · CMF_z(lambda) deltaLambda }
//          
//           where phi(lambda) is either spectral irradiance E(lambda) or spectral radiance, L(lambda)
//           
//          Devices that measure the colorimetric values of sources, such as spectroradiometers and colorimeters, do not report X, Y, Z, rather they report x, y, Y.
//          This separates the chromatic and achromatic information, since these two parameters can easily be varied independently.
//          Tristimulus value Y has units of either lx (from irradiance) or cd/m2 (from radiance).
//           
//                           ***End of citation***
//
//          Then, we have to integrate XYZ, compute chromaticities xy and convert into RGB using a matrix. After making any necessary RGB calibration (clipping, mixing white, normalizing to brigthen up...)
//          we can boost our output based on Y, as this is actually a measure of light
//
//------------------------------------------------------------------------------------------------------------------------------------------------------|
//                                                                                                                                                      |
//     Conversion matrices between different color spaces.                                                                                              |
//     These matrices assume that XYZ are in nominal range and a standard D65 white point illuminant (T = 6504K)                                        |
//                                                                                                                                                      |
//     For a specialized conversion matrix between a with different white point, consult:                                                               |
//                                                                                                                                                      |
//                   http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html                                                                   |
//                                                                                                                                                      |
//     A chromatic adaptation algorith may also be adopted to convert between different reference whites                                                |
//                                                                                                                                                      |
//                   http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html                                                                   |
//                                                                                                                                                      |
//------------------------------------------------------------------------------------------------------------------------------------------------------|

//************************** Conversion between XYZ tristimulus values and RGB********************************//
static const float3x3 XYZ_TO_LINEAR_RGB =
{
    + 3.2406255f, - 1.5372080f, - 0.4986286f,
    - 0.9689307f, + 1.8757561f, + 0.0415175f,
    + 0.0557101f, - 0.2040211f, + 1.0569959f
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

//************************** Conversion between xyz chromaticities and RGB********************************//

//static const float3x3 xyz_TO_LINEAR_RGB =
//{
//};
//
//static const float3x3 LINEAR_RGB_TO_xyz = 
//{
//
//};

//Contains chromaticities of a specific color system;
//i.e, sRGB: 
//    Red        : x{0.64]  , y{0.33}
//    Green      : x{0.30]  , y{0.60}
//    Blue       : x{0.15]  , y{0.06}
//    White(D65) : x{0.3127], y{0.3290}
//

struct ColorSystem//(32 bytes)
{
    float2 red;
    float2 green;
    float2 blue;
    float2 white;
};
float3x3 compute_XYZ_to_RGB_matrix(in const ColorSystem colorSys)
{
    return 0;
}

float3x3 compute_xyz_to_RGB_matrix(in const ColorSystem colorSys)
{
    return 0;
}
//---------------------------------------------
//
//    Clips RGB values in the domain [0, 1]
//
//
//---------------------------------------------
float3 clipRGB(in const float3 RGB)
{
    return clamp(RGB, 0.0f, 1.0f);
}

//---------------------------------------------//
//                                             //
//    Clips RGB negative values to zero        //
//                                             //
//                                             //
//---------------------------------------------//
float3 clipRGB_clamp_neg(in const float3 RGB)
{
    return max(RGB, 0.0f);
}


//https://aty.sdsu.edu/explain/optics/rendering.html
//           First, note that the white point of the monitor corresponds to (R, G, B) = (1, 1, 1). 
//           Then, if the value of some requested component — G, for example — is negative, we can bring the point back from the spectrum locus to the BR edge of the gamut triangle by 
//           multiplying its offset from the white point by 1/(1 - G). (As G is assumed to be negative here, the denominator of the fraction is bigger than unity, so the fraction is less than unity.)
//             
//          Actually, we want to preserve relative brightnesses; so instead of moving toward white, we need to shrink the out-of-range vector toward the gray point with the same brightness, Y, as the point in the spectrum we are simulating. 
//          That means that the correction factor is Y/(Y - G). Call this correction factor F; then the corrected G intensities are Ggamut limit = Y + F * (Gspectrum - Y)
float3 clipRGB_add_white_preserveBrightness(in const float3 RGB, in const float Y)
{
    float3 F = Y / (Y - RGB);
    
    return Y + min(F, 1.0f) * (RGB - Y);
}


//---------------------------------------------------------------------------//
//                                                                           //
//    Adds enough white to a given RGB color to make it nonnegative          //
//                                                                           //
//                                                                           //
//---------------------------------------------------------------------------//
float3 clipRGB_add_white(in const float3 RGB)
{
    //Find smallest negative component(clips to zero if no negative component)
    //Make it positive and apply to every component
    float w = -min(min(RGB.r, min(RGB.g, RGB.b)), 0.0f);
    
    return RGB + float3(w, w, w);
}

float3 normalize_RGB(in const float3 RGB)
{
    float m = max(RGB.r, max(RGB.g, RGB.b)) + 1e-4;
    
    return RGB / m;
}
#endif //_COLOR_SPACE_HLSLI
