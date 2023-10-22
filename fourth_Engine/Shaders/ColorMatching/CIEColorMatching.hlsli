#ifndef _CIE_COLOR_MATCHING_HLSLI
#define _CIE_COLOR_MATCHING_HLSLI

//NVidia: Simple Analytic Approximations to the CIE XYZ Color Matching Functions
//https://research.nvidia.com/publication/2013-07_simple-analytic-approximations-cie-xyz-color-matching-functions 
//https://jcgt.org/published/0002/02/01/paper.pdf

float X_Fit_1931(in const float wavelength) //in nanometers
{
     float t1 = (wavelength - 442.0f) * (wavelength < 442.0f) ?    0.0624f : 0.0374f;
     float t2 = (wavelength - 599.8f) * (wavelength < 599.8f) ?    0.0264f : 0.0323f;
     float t3 = (wavelength - 501.1f) * (wavelength < 501.1f) ?    0.0490f : 0.0382f;

     return 0.362f * exp(-0.5f * t1 * t1) + 1.056f * exp(-0.5f * t2 * t2) - 0.065f * exp(-0.5f * t3 * t3);
     
}

float Y_Fit_1931(in const float wavelength) //in nanometers
{
     float t1 = (wavelength - 568.8f) * (wavelength < 568.8f) ?    0.0213f : 0.0247f;
     float t2 = (wavelength - 530.9f) * (wavelength < 530.9f) ?    0.0613f : 0.0322f;

     return 0.362f * exp(-0.5f * t1 * t1) + 1.056f * exp(-0.5f * t2 * t2);
     
}

float Z_Fit_1931(in const float wavelength) //in nanometers
{
     float t1 = (wavelength - 437.0f) * (wavelength < 437.0f) ?    0.0845f : 0.0278f;
     float t2 = (wavelength - 459.0f) * (wavelength < 459.0f) ?    0.0385f : 0.0725f;

     return 0.362f * exp(-0.5f * t1 * t1) + 1.056f * exp(-0.5f * t2 * t2);
     
}

//CIE D65 transformations

//Converts cromaticity XYZ values to RGB linear (not sRGB)
float3 XYZ_to_RGB(in const float3 XYZ)
{
     const float3x3 CMF = float3x3(
          float3( 3.2404542f, -1.5371385f, -0.4985314f),
          float3(-0.9692660f,  1.8760108f,  0.0415560f),
          float3( 0.0556434f, -0.2040259f,  1.0572252f));

     return mul(XYZ, CMF);
}

//Converts linear RGB to cromaticity XYZ
float3 RGB_to_XYZ(in const float3 RGB)
{
     const float3x3 CMF = float3x3(
          float3( 0.4124564f,  0.3575761f,  0.1804375f),
          float3( 0.2126729f,  0.7151522f,  0.0721750f),
          float3( 0.0193339f,  0.1191920f,  0.9503041f));
     
     return mul(RGB, CMF);
}

//The D illuminant can be used to approximate the daylight spectrum between 4000K and 25,000K. 
//The inputs needed are the xy(not XYZ) coordinates from the 1931 CIE coordinate system, along with the three functions named S0 (blue), S1 (green) and S2 (red)

void compute_D_Illuminant_M1M2(in const float2 CIEcoords, inout float M1, inout float M2)
{
     float invDenominator = 1.0f / (0.0241f + 0.2562 * CIEcoords.x - 0.7341f * CIEcoords.y);
     M1 = (-1.3515f  -  1.7703f * CIEcoords.x +  5.9114f * CIEcoords.y) * invDenominator;
     M2 = ( 0.0300f - 31.4424f * CIEcoords.x + 30.0717f * CIEcoords.y) * invDenominator;
}

float spectrum_Dxy_Illuminant(in const float M1, in const float M2, in const float S0, in const float S1, in const float S2)
{
    return S0 + S1 * M1 + S2 * M2;
}

float computeColorCorrelatedTemperatureFromCromaticity(in const float2 xy)
{
     const float xe = 0.3320f;
     const float ye = 0.1858f;
     const float n = (xy.x - xe)/(xy.y - ye);

     const float n2 = n * n;
     const float n3 = n2 * n;

     return -449.0f * n3 + 3525.0f * n2 - 6823.3f * n + 5520.33f;
}

//---------D illuminant S0, S1, S2 curves. Each index represents a wavelength, each step is 5nm. If a value gets past or under the [360, 780] range, it gets clamped.
//wavelength start: 380nm
//wavelength end:   780nm

//TOOD: we could make a triple sample by interpolating bewteen wave_n-1, wave_n, wave_n+1
uint getIlluminantD_wavelengthIndex(in const float wavelength)
{
    const float maxWave = 780;
    const float minWave = 380;
    const uint waveStep = 5;

    uint index = clamp((wavelength - minWave) / waveStep, minWave, maxWave);

    return index;
}

static const float3 D_S012[80] = 
{ float3( 63.40f,  38.50f, 3.0f),
  float3( 64.60f,  36.75f,	 2.10f),
  float3( 65.80f,  35.00f,	 1.20f),
  float3( 80.30f,  39.20f,	 0.05f),
  float3( 94.80f,  43.40f,	-1.10f),
  float3( 99.80f,  44.85f,	-0.80f),
  float3(104.80f,   6.30f,	-0.50f),
  float3(105.3f ,  45.10f,  -0.60f),
  float3(105.9f ,  43.90f,  -0.70f),
  float3(101.3f ,  40.50f,  -0.95f),
  float3(96.80f ,  37.10f,  -1.20f),
  float3(105.3f ,  36.90f,  -1.90f),
  float3(113.9f ,  36.70f,  -2.60f),
  float3(119.7f ,  36.30f,  -2.75f),
  float3(125.6f ,  35.90f,  -2.90f),
  float3(125.5f ,  34.25f,  -2.85f),
  float3(125.5f ,  32.60f,  -2.80f),
  float3(123.4f ,  30.25f,  -2.70f),
  float3(121.3f ,  27.90f,  -2.60f),
  float3(121.3f ,  26.10f,  -2.60f),
  float3(121.3f ,  24.30f,  -2.60f),
  float3(117.4f ,  22.20f,  -2.20f),
  float3(113.5f ,  20.10f,  -1.80f),
  float3(113.3f ,  18.15f,  -1.65f),
  float3(113.1f ,  16.20f,  -1.50f),
  float3(111.9f ,  14.70f,  -1.40f),
  float3(110.8f ,  13.20f,  -1.30f),
  float3(108.6f ,  10.90f,  -1.25f),
  float3(106.5f ,  8.60f,   -1.20f),
  float3(107.6f ,  7.35f,   -1.10f),
  float3(108.8f ,  6.10f,   -1.00f),
  float3(107.0f ,  5.15f,   -0.75f),
  float3(105.3f ,  4.20f,   -0.50f),
  float3(104.8f ,  3.05f,   -0.40f),
  float3(104.4f ,  1.90f,   -0.30f),
  float3(102.2f ,  0.95f,   -0.15f),
  float3(100.0f ,  0.00f,    0.00f),
  float3( 98.00f,  -0.80f,   0.10f  ),
  float3( 96.00f,  -1.60f,   0.20f  ),
  float3( 95.55f,  -2.55f,   0.35f  ),
  float3( 95.10f,  -3.50f,   0.50f  ),
  float3( 92.10f,  -3.50f,   1.30f  ),
  float3( 89.10f,  -3.50f,   2.10f  ),
  float3( 89.80f,  -4.65f,   2.65f  ),
  float3( 90.50f,  -5.80f,   3.20f  ),
  float3( 90.40f,  -6.50f,   3.65f  ),
  float3( 90.30f,  -7.20f,   4.10f  ),
  float3( 89.35f,  -7.90f,   4.40f  ),
  float3( 88.40f,  -8.60f,   4.70f  ),
  float3( 86.20f,  -9.05f,   4.90f  ),
  float3( 84.00f,  -9.50f,   5.10f  ),
  float3( 84.55f, -10.20f,   5.90f  ),
  float3( 85.10f, -10.90f,   6.70f  ),
  float3( 83.50f, -10.80f,   7.00f  ),
  float3( 81.90f, -10.70f,   7.30f  ),
  float3( 82.25f, -11.35f,   7.95f  ),
  float3( 82.60f, -12.00f,   8.60f  ),
  float3( 83.75f, -13.00f,   9.20f  ),
  float3( 84.90f, -14.00f,   9.80f  ),
  float3( 83.10f, -13.80f,   10.00f ),
  float3( 81.30f, -13.60f,   10.20f ),
  float3( 76.60f, -12.80f,   9.25f  ),
  float3( 71.90f, -12.00f,   8.30f  ),
  float3( 73.10f, -12.65f,   8.95f  ),
  float3( 74.30f, -13.30f,   9.60f  ),
  float3( 75.35f, -13.10f,   9.05f  ),
  float3( 76.40f, -12.90f,   8.50f  ),
  float3( 69.85f, -11.75f,   7.75f  ),
  float3( 63.30f, -10.60f,   7.00f  ),
  float3( 67.50f, -11.10f,   7.30f  ),
  float3( 71.70f, -11.60f,   7.60f  ),
  float3( 74.35f, -11.90f,   7.80f  ),
  float3( 77.00f, -12.20f,   8.00f  ),
  float3( 71.10f, -11.20f,   7.35f  ),
  float3( 65.20f, -10.20f,   6.70f  ),
  float3( 56.45f,  -9.00f,   5.95f  ),
  float3( 47.70f,  -7.80f,   5.20f  ),
  float3( 58.15f,  -9.50f,   6.30f  ),
  float3( 68.60f, -11.20f,   7.40f  ),
  float3( 66.80f, -10.80f,   7.10f  ),
  float3( 65.00f, -10.40f,   6.80f  )
};



#endif //_CIE_COLOR_MATCHING_HLSLI