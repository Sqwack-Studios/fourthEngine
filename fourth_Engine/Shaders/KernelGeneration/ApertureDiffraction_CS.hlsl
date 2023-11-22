#ifndef _APERTURE_DIFFRACTION_CS
#define _APERTURE_DIFFRACTION_CS

#define NUM_CHANNELS 1

#include "../Compute/FFTs.hlsli"
//During first pass, SRV is apertureFunction, UAV is output complex
//During second pass, SRV is output complex, UAV is powerDensity
Texture2D<float4>    apertureFunction : register(SRV_PP_0);
RWTexture2D<float4>      powerDensity : register(UAV_PP_COMPUTE_0);
RWStructuredBuffer<float>         DC  : register(UAV_PP_COMPUTE_1);

cbuffer apertureUniform : register(CB_PP_0)
{
    uint   g_fftFlags;
    float  g_spatialPeriod; //equal to sampling period
    float  g_spatialFrequency;
    float  g_wavelength;
    //
    float  g_focalDistance;
    float2 g_uvShift;
    float  g_apertureSize;
}

[numthreads(WORKGROUP_SIZE_X, 1, 1)]
void main( uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    Complex threadBuffer[NUM_CHANNELS][RADIX];
    const bool isHorizontal   = g_fftFlags & 0x01;
    const bool isForward      = g_fftFlags & 0x02;
    const bool isFirstPass    = g_fftFlags & 0x04;
    const bool useCustomScale = g_fftFlags & 0x08;
    const bool shift          = g_fftFlags & 0x10;

    const uint localThreadID = tid.x;
    const uint scanLine = gid.x;
    const uint dataStride = STRIDE;

    loadSrcToLocalBuffer(threadBuffer, isHorizontal, scanLine, localThreadID, dataStride, apertureFunction);


    sharedFFT(SIGNAL_LENGTH, localThreadID, isForward, threadBuffer);
    
    if (!isFirstPass)
    {
        {
            [unroll(RADIX)]
            for (uint r = 0; r < RADIX; ++r)
            {
                threadBuffer[0][r] = complexMult(threadBuffer[0][r], complexConjugate(threadBuffer[0][r]));
            }
        }

         if(scanLine == 0 && localThreadID == 0)
         {
              DC[0] = 1.0f/threadBuffer[0][0].x; // first value of the resulting signal is the DC value
         }
       
    }

    saveLocalBufferToDestination(threadBuffer, isHorizontal, shift && !isFirstPass, scanLine, localThreadID, dataStride, g_uvShift, powerDensity);

    
}

#endif //_APERTURE_DIFFRACTION_CS