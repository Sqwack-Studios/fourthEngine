#include "../Common.hlsli"

#ifndef _FFT_STOCKHAM_HLSLI_
#define _FFT_STOCKHAM_HLSLI_

#if _FFT_SHARED_MODEL == _FFT_ShMem_LOOP_CHANNELS
struct sharedFFTData
{
    float red   [1024];
};

#endif

#if _FFT_SHARED_MODEL == _FFT_ShMem_COMPUTE_RGB

struct sharedFFTData
{
    float red   [1024];
    float green [1024];
    float blue  [1024];
};

#endif

groupshared sharedFFTData sharedfft;
float2 registerCache[2];
static const uint THREADGROUP_SIZE_X = 512;
static const uint RADIX_SIZE = 2;
static const uint RADIX_SIZE_LOG = 1;

//FWD FFT; input a HDR RGBA_16 texture 
//FFT outputs float3 Real and Imaginary part, which are organized as two textures:
//float4 -> RGB = Real ; A = Imaginary.x (RGBA_32)
//float2 -> RG = Imaginary.yz; (RG_32)

struct FFTForwardInout 
{
    RWTexture2D<float4> input;
    RWTexture2D<float4> ReRGBImA;
    RWTexture2D<float2> ImRG;
};

struct FFTConvolute
{
    
};

struct FFTInverseInout
{
    
};
uint expand(in uint idxL, in uint N1, in uint N2)
{
    return (idxL / N1) * N1 * N2 + (idxL % N1);
}

void exchange(in uint stride, in uint idxD, in uint incD, in uint idxS, in uint incS)
{
    uint shReIdx = 0;
    uint shImIdx = shReIdx + THREADGROUP_SIZE_X * RADIX_SIZE;
    
    GroupMemoryBarrierWithGroupSync();
    
    uint inoutIdx;
    [unroll]
    for (uint r1 = 0; r1 < RADIX_SIZE; ++r1)
    {
        inoutIdx = (idxD + incD << RADIX_SIZE_LOG) * stride;
        sharedfft.red[shReIdx + inoutIdx] = registerCache[r1];
        sharedfft.red[shImIdx + inoutIdx] = registerCache[r1];
        
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    [unroll]
    for (uint r2 = 0; r2 < RADIX_SIZE; ++r2)
    {
        inoutIdx = (incS + incS << RADIX_SIZE_LOG) * stride;
        registerCache[r2] = 
        float2(sharedfft.red[shReIdx + inoutIdx], 
               sharedfft.red[shImIdx + inoutIdx]);

    }

}

void stockhamShared_Forward(in uint N, in uint Radix, in uint RadixLog, in uint j, in uint stride = 1)
{
    const uint NdivR = N >> RadixLog;

    [unroll]
    for (uint Ns = 1; Ns < N; Ns << RadixLog)
    {
        float angle = -_2PI * (j % Ns) / (Ns << RadixLog);

        for (uint r = 0; r < Radix; ++r)
        {
            float sin, cos;
            sincos(angle * r, sin, cos);
            
            registerCache[r] *= float2(cos, sin);
            
        }

        //butterfly pass
        float2 a0 = registerCache[0];
        registerCache[0] = a0 + registerCache[1];
        registerCache[1] = a0 - registerCache[1];

        uint idxD = expand(j, Ns, Radix);
        uint idxS = expand(j, NdivR, Radix);
        
        exchange(stride, idxD, Ns, idxS, NdivR);
    }

}




#endif //FFT_STOCKHAM_HLSLI