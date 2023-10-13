#include "../Common.hlsli"

#ifndef _FFT_STOCKHAM_HLSLI_
#define _FFT_STOCKHAM_HLSLI_

#define Complex float2

Complex complexMult(in Complex a, in Complex b)
{
    return Complex(a.x * b.x - a.y * b.y, a.x * b.y + b.x * a.y);
}

Complex complexConjugate(in Complex a)
{
    return Complex(a.x, -a.y);
}


groupshared float sharedfft[2 * SIGNAL_LENGTH];

//FWD FFT; input a HDR RGBA_16 texture 
//FFT outputs float3 Real and Imaginary part, which are organized as two textures:
//float4 -> RGB = Real ; A = Imaginary.x (RGBA_32)
//float2 -> RG = Imaginary.yz; (RG_32)

struct FFTForwardInoutHorizontal 
{
    RWTexture2D<float4> input;
    RWTexture2D<float4> ReRGBImA;
    RWTexture2D<float2> ImRG;
};

struct FFTForwardInoutVertical
{
    RWTexture2D<float4> ReRGBImA;
    RWTexture2D<float2> ImRG;
};
struct FFTConvolute
{
    Texture2D<float2>   convWeights;
    RWTexture2D<float4> ReRGBImA;
    RWTexture2D<float2> ImRG;
};

struct FFTInverseInoutVertical
{
    RWTexture2D<float4> ReRGBImA;
    RWTexture2D<float2> ImRG;
};

struct FFTInverseInoutHorizontal
{
    RWTexture2D<float4> output;
    RWTexture2D<float4> ReRGBImA;
    RWTexture2D<float2> ImRG;
};

uint expand(in uint idxL, in uint N1, in uint N2)
{
    return (idxL / N1) * N1 * N2 + (idxL % N1);
}

//https://developer.nvidia.com/blog/using-shared-memory-cuda-cc/
//To minimize bank conflicts, it is important to understand how memory addresses map to memory banks. 
//Shared memory banks are organized such that successive 32-bit words are assigned to successive banks and the bandwidth is 32 bits per bank per clock cycle. 
//For devices of compute capability 1.x, the warp size is 32 threads and the number of banks is 16. 
//A shared memory request for a warp is split into one request for the first half of the warp and one request for the second half of the warp. 
//Note that no bank conflict occurs if only one memory location per bank is accessed by a half warp of threads.

//For devices of compute capability 2.0, the warp size is 32 threads and the number of banks is also 32. 
//A shared memory request for a warp is not split as with devices of compute capability 1.x, 
//meaning that bank conflicts can occur between threads in the first half of a warp and threads in the second half of the same warp.
#define LDS_BANKS 32

void exchangeBankConflictOptimized()
{
    
}

void exchange(in uint stride, in uint idxD, in uint incD, in uint idxS, in uint incS, inout Complex threadBuffer[RADIX])
{
    uint shReIdx = 0;
    uint shImIdx = shReIdx + WORKGROUP_SIZE_X * RADIX;
    
    
    GroupMemoryBarrierWithGroupSync();
    
    uint inoutIdx;
    [unroll]
    for (uint r1 = 0; r1 < RADIX; ++r1)
    {
        inoutIdx = (idxD + incD << RADIX_LOG) * stride;
        sharedfft[shReIdx + inoutIdx] = threadBuffer[r1].x;
        sharedfft[shImIdx + inoutIdx] = threadBuffer[r1].y;
        
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    [unroll]
    for (uint r2 = 0; r2 < RADIX; ++r2)
    {
        inoutIdx = (incS + incS << RADIX_LOG) * stride;
        threadBuffer[r2] =
        Complex(sharedfft[shReIdx + inoutIdx], 
               sharedfft[shImIdx + inoutIdx]);

    }

}

void FFT_Radix_2(inout Complex threadBuffer[RADIX])
{
    Complex temp = threadBuffer[0];
    threadBuffer[0] = temp + threadBuffer[1];
    threadBuffer[1] = temp - threadBuffer[1]; // double subtraction of index 1 is another option
}
void FFT_Radix_R(in bool isForward, inout Complex threadBuffer[RADIX])
{
#if RADIX == 2
    FFT_Radix_2(threadBuffer);
#endif

#if RADIX == 4
    FFT_Radix_4(isForward, threadBuffer[0], threadBuffer[1], threadBuffer[2], threadBuffer[3]);
#endif

#if RADIX == 8
    FFT_Radix_8(isForward, threadBuffer[0], threadBuffer[1], threadBuffer[2], threadBuffer[3],
                           threadBuffer[4], threadBuffer[5], threadBuffer[6], threadBuffer[7])
#endif
}
void twiddle(in const bool isForward, inout Complex threadBuffer[RADIX], in const uint threadIdx, in const uint N)
{
    const float angle = isForward ? 
    -TWO_PI * float(threadIdx % N) / float(N << RADIX_LOG) : //isForward
     TWO_PI * float(threadIdx % N) / float(N << RADIX_LOG);  //not forward

    Complex twiddleBase;
    sincos(angle, twiddleBase.y, twiddleBase.x);
    
    Complex twiddle = twiddleBase;
    for (uint r = 1; r < RADIX; ++r)
    {
        threadBuffer[r] = complexMult(twiddle, threadBuffer[r]); //index 0 access and multiplication is useless because its power 0
        twiddle = complexMult(twiddle, twiddleBase);
    }

}

//Real-2-Complex
void stockhamShared_Forward(in const uint N, in const uint threadIdx, in const bool isForward, inout Complex threadBuffer[RADIX])
{
    const uint numGroups = N / RADIX;
    const uint idxS = expand(threadIdx, numGroups, RADIX); // -> this number is always equal to threadIdx because (thread / numGroups) = 0, and %(thread / cols) = thread. thread [0, NUM_GROUPS - 1]

    [unroll]
    for (uint Ns = 1; Ns < N; Ns << RADIX_LOG)
    {
        twiddle(isForward, threadBuffer, threadIdx, Ns);

        FFT_Radix_R(isForward, threadBuffer);

        uint idxD = expand(threadIdx, Ns, RADIX);
        
        exchange(1, idxD, Ns, idxS, numGroups, threadBuffer);
    }

}

void loadForwardHorizontal()
{
    
}

void loadForwardVertical()
{
    
}

void loadConvolutionStage()
{
    
}

void loadInverseHorizontal()
{
    
}

void loadInverseVertical()
{
    
}

#endif //FFT_STOCKHAM_HLSLI