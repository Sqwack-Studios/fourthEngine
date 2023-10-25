#ifndef _DXY_ILLUMINANT_HELPERS_HLSLI_
#define _DXY_ILLUMINANT_HELPERS_HLSLI_

//D65 white point CIE1931 chromaticity coordinates
#define CIE1931_D65_xy float2(0.31271f, 0.32902f)

//D65 integration result of the D65 relative spectral distribution from [380, 780] 1nm binned
#define CIE1931_D65_INTEGRATION 35180.2319666f
#define CIE1931_D65_INVINTEGRATION 0.000028425f


#define LUMINOUS_EFFICACY 653.002f //lumens/W
//#define CIE1931_D65_x 0.31271f      
//#define CIE1931_D65_y 0.32902f    


//The D illuminant can be used to approximate the daylight spectrum between 4000K and 25,000K. Based on what we consider our white point, the spectrum changes, and also XYZ->RGB relationship.
//The inputs needed are the xy(not XYZ) coordinates from the 1931 CIE coordinate system, along with the three functions named S0 (blue), S1 (green) and S2 (red)


//From "Measuring Color" by R.W.G Hunt, M.R. Pointer:
/*
         For daylight illuminants, CIE Standard Illuminant D65 should be used whenever possible;
         if D65 is not appropriate, then either ID65, D50, ID50, D55, or D75, should be used if
         possible. If none of these daylight illuminants is appropriate, then one of the other CIE D
         illuminants, defined below, should be used

         The relative spectral power distributions, S(l), of the D illuminants are given by:

                                 S(l) = S0(l) +M1S1(l) +M2S2(l)

         where S0(l), S1(l), S2(l) are functions of wavelength, l, as given in the table below,and
         M1,M2 are factors whose values are related to the chromaticity co-ordinates xD, yD as
         follows:

                      M1 = (-1.3515 - 1.7703xD + 5.9114yD)/(0.0241 + 0.2562xD - 0.7341yD)
                      M2 = (0.0300 - 31.4424xD + 30.0717yD)/(0.0241 + 0.2562xD - 0.7341yD)

         A white point to be considered part of a D illuminant must verify that the chromaticity coords: 
                      
                             y = -3.000x^2 + 2.870x - 0.275, with x [0.250, 0.380]

*/

/*------------------------------------------------------------------------------------------------------------------------------//
|  
|  computes the M1, M2 weights from a white point's CIE chromaticity coordinates. Use it to compute the RSP of a D illuminant
|
|  IN   : float (CIE coords) Acceptable ranges are specified above. Use the function "computeColorCorrelatedTemperatureFromCromaticity" to compute correct xy values.
|  INOUT: float( M1 ) : weights to be provided into "spectrum_Dxy_Illuminant" to compute the RSD
|         float( M2 ) :
|---------------------------------------------------------------------------------------------------------------------------------*/ 

void compute_D_Illuminant_M1M2(in const float2 CIEcoords, inout float M1, inout float M2)
{
    float invDenominator = 1.0f / (0.0241f + 0.2562 * CIEcoords.x - 0.7341f * CIEcoords.y);
    M1 = (-1.3515f - 1.7703f * CIEcoords.x + 5.9114f * CIEcoords.y) * invDenominator;
    M2 = (0.0300f - 31.4424f * CIEcoords.x + 30.0717f * CIEcoords.y) * invDenominator;
}

/*------------------------------------------------------------------------------------------------------------------------------//
|  
|  computes the chromaticity of a whitepoint daylight illuminant (D illuminant) from a color temperature
|
|  IN : float (temperature) [4000, 25000] Kelvin. Any value will be clamped to that range.
|  OUT: float2( xy chromaticities ) : xy chromaticities of a D illuminant whitepoint
|
|---------------------------------------------------------------------------------------------------------------------------------*/ 
float2 temperature_to_illuminantD_chromaticity(in float temperature)
{
    float2 xy;
    
    //minTemp = 4000.0f;
    //maxTemp = 25000.0f;
    
    temperature = clamp(temperature, 4000.0f, 25000.0f);
    
    float invT1 = 1e+3f / temperature;
    float invT2 = invT1 * invT1;
    float invT3 = invT2 * invT1;
    
    if (temperature <= 7000.0f)
    {
        xy.x = -4.6070f * invT3 + 2.9678f * invT2 + 0.09911f * invT1 + 0.244063f;
    }
    else
    {
        xy.y = -2.0064f * invT3 + 1.9018f * invT2 + 0.24748f * invT1 + 0.23704f;
    }

    xy.y = -3.0f * xy.x * xy.x + 2.87f * xy.x - 0.275f;

    return xy;
}

float computeColorCorrelatedTemperatureFromCromaticity(in const float2 xy)
{
    const float xe = 0.3320f;
    const float ye = 0.1858f;
    const float n = (xy.x - xe) / (xy.y - ye);

    const float n2 = n * n;
    const float n3 = n2 * n;

    return -449.0f * n3 + 3525.0f * n2 - 6823.3f * n + 5520.33f;
}


float spectrum_Dxy_Illuminant(in const float M1, in const float M2, in const float S0, in const float S1, in const float S2)
{
    return S0 + S1 * M1 + S2 * M2;
}



//TODO: float2 computeCromaticityFromColorCorrelatedTemperature(in const float CCT)

//---------D illuminant S0, S1, S2 curves. Each index represents a wavelength, each step is 5nm. If a value gets past or under the [360, 780] range, it gets clamped.
//wavelength start: 380nm
//wavelength end:   780nm

//TOOD: we could make a triple sample by interpolating bewteen wave_n-1, wave_n, wave_n+1
uint getIlluminantD_wavelengthIndex(in const float wavelength)
{
    const float maxWave = 780.0f;
    const float minWave = 380.0f;
    const uint waveStep = 5;

    uint index = clamp((wavelength - minWave) / waveStep, 0, 80);

    return index;
}


/* Relative spectral distribution weights of the D illuminants */
//We employ these values to compute a RSP of a illuminant with a given color temperature.
//Computing the RSP everytime for the same color temperature is overkill, but it is a price
//to pay if we want to modify color temperature on the fly.

static const float3 D_S012[81] =
{
  float3(63.40f, 38.50f, 3.0f),
  float3(64.60f, 36.75f, 2.10f),
  float3(65.80f, 35.00f, 1.20f),
  float3(80.30f, 39.20f, 0.05f),
  float3(94.80f, 43.40f, -1.10f),
  float3(99.80f, 44.85f, -0.80f),
  float3(104.80f, 6.30f, -0.50f),
  float3(105.35f, 45.10f, -0.60f),
  float3(105.90f, 43.90f, -0.70f),
  float3(101.35f, 40.50f, -0.95f),
  float3(96.80f, 37.10f, -1.20f),
  float3(105.35f, 36.90f, -1.90f),
  float3(113.90f, 36.70f, -2.60f),
  float3(119.75f, 36.30f, -2.75f),
  float3(125.60f, 35.90f, -2.90f),
  float3(125.55f, 34.25f, -2.85f),
  float3(125.50f, 32.60f, -2.80f),
  float3(123.40f, 30.25f, -2.70f),
  float3(121.30f, 27.90f, -2.60f),
  float3(121.30f, 26.10f, -2.60f),
  float3(121.30f, 24.30f, -2.60f),
  float3(117.40f, 22.20f, -2.20f),
  float3(113.50f, 20.10f, -1.80f),
  float3(113.30f, 18.15f, -1.65f),
  float3(113.10f, 16.20f, -1.50f),
  float3(111.95f, 14.70f, -1.40f),
  float3(110.80f, 13.20f, -1.30f),
  float3(108.65f, 10.90f, -1.25f),
  float3(106.50f, 8.60f, -1.20f),
  float3(107.65f, 7.35f, -1.10f),
  float3(108.80f, 6.10f, -1.00f),
  float3(107.05f, 5.15f, -0.75f),
  float3(105.30f, 4.20f, -0.50f),
  float3(104.85f, 3.05f, -0.40f),
  float3(104.40f, 1.90f, -0.30f),
  float3(102.20f, 0.95f, -0.15f),
  float3(100.00f, 0.00f, 0.00f),
  float3(98.00f, -0.80f, 0.10f),
  float3(96.00f, -1.60f, 0.20f),
  float3(95.55f, -2.55f, 0.35f),
  float3(95.10f, -3.50f, 0.50f),
  float3(92.10f, -3.50f, 1.30f),
  float3(89.10f, -3.50f, 2.10f),
  float3(89.80f, -4.65f, 2.65f),
  float3(90.50f, -5.80f, 3.20f),
  float3(90.40f, -6.50f, 3.65f),
  float3(90.30f, -7.20f, 4.10f),
  float3(89.35f, -7.90f, 4.40f),
  float3(88.40f, -8.60f, 4.70f),
  float3(86.20f, -9.05f, 4.90f),
  float3(84.00f, -9.50f, 5.10f),
  float3(84.55f, -10.20f, 5.90f),
  float3(85.10f, -10.90f, 6.70f),
  float3(83.50f, -10.80f, 7.00f),
  float3(81.90f, -10.70f, 7.30f),
  float3(82.25f, -11.35f, 7.95f),
  float3(82.60f, -12.00f, 8.60f),
  float3(83.75f, -13.00f, 9.20f),
  float3(84.90f, -14.00f, 9.80f),
  float3(83.10f, -13.80f, 10.00f),
  float3(81.30f, -13.60f, 10.20f),
  float3(76.60f, -12.80f, 9.25f),
  float3(71.90f, -12.00f, 8.30f),
  float3(73.10f, -12.65f, 8.95f),
  float3(74.30f, -13.30f, 9.60f),
  float3(75.35f, -13.10f, 9.05f),
  float3(76.40f, -12.90f, 8.50f),
  float3(69.85f, -11.75f, 7.75f),
  float3(63.30f, -10.60f, 7.00f),
  float3(67.50f, -11.10f, 7.30f),
  float3(71.70f, -11.60f, 7.60f),
  float3(74.35f, -11.90f, 7.80f),
  float3(77.00f, -12.20f, 8.00f),
  float3(71.10f, -11.20f, 7.35f),
  float3(65.20f, -10.20f, 6.70f),
  float3(56.45f, -9.00f, 5.95f),
  float3(47.70f, -7.80f, 5.20f),
  float3(58.15f, -9.50f, 6.30f),
  float3(68.60f, -11.20f, 7.40f),
  float3(66.80f, -10.80f, 7.10f),
  float3(65.00f, -10.40f, 6.80f)
};


#endif //_DXY_ILLUMINANT_HELPERS_HLSLI_