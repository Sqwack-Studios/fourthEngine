//
//
//
//
//
//
//
//
//
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////
//RWTexture2D<float4> input : register(UAV_PP_COMPUTE_0);
//RWTexture2D<float4> realRGBimagA : register(UAV_PP_COMPUTE_1);
//RWTexture2D<float2> imagRG : register(UAV_PP_COMPUTE_2);
//Texture2D<float>    kernel : register(SRV_PP_0);

//static const uint FLAG_HORIZONTAL = 1;
//static const uint MODE_FORWARD = 2;
//static const uint MODE_CONVOLUTE = 4;
//static const uint MODE_INVERSE = 8;

#define SIGNAL_LENGTH 1024
#define RADIX 2
#define RADIX_LOG 1 
#define WORKGROUP_SIZE_X (SIGNAL_LENGTH / RADIX)

#include "FFT_Stockham.hlsli"

cbuffer fftImageDsc : register(CB_PP_0)
{
	 uint2 g_fft_size;
	 float g_log2_fft_size;
	 uint g_isForward;
}

void copySourceRealToLocalBuffers()
{
	
}

void copyLocalBuffersToDestination()
{
	
}

#define NUM_CHANNELS 3

//TODO: Actually, R2C, C2C, C2R are computing the exact same thing. Optimizations have to be made. In order to get a performance
//boost if we know that our signal is purely real, or that our destination signall will be purely real, we have to take advantage 
//of symmetry properties. We can pack each pair of signal values (x +iy) in a single channel. For a RGBA signal, we can have Real and 
//imaginary parts in a signe channel, halving our memory consuption and reducing bandwitch. Extra computations need to be made to pack/unpack
//data, but now we will be doing half of the calculations, making effectively x2 faster.

//#ifdef FFT_R2C
#if 1

///////////////////////////////////////////////////////////////////////////////////////////
//Computes a Stockham LDS FFT Real-To-Complex. Performs three FFTs from a source texture.
//
//TODO: We are dropping support to apply a FFT to the alpha channel, as this would generate (x + iy) * 4 channels.
//We would need two full fp32 textures and might hurt bandwitch. I was planning to load the entire FFT in registers in one go.
//By adding another channel, we might get register overuse. I have not made any calculation nor profiling, but I wanna get it working.
//
//SrcTexture -> a float4 color texture
//DstTexture -> a float4 fp32 (r, g, b) -> Real Part, (a) -> Imaginary Part of the R channel
//           -> a float2 fp32 (r, g) are the rest of the imaginary components
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////

Texture2D<float4>   srcSignal        : register(SRV_PP_0);
RWTexture2D<float4> dstRe_rgb_Im_r   : register(UAV_PP_0);
RWTexture2D<float2> dstIm_gb         : register(UAV_PP_0);


[numthreads(WORKGROUP_SIZE_X, 1, 1)]
void stockhamSharedR2CFFT(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{

	Complex threadBuffer[NUM_CHANNELS][RADIX];
}

#endif

#if 1

Texture2D<float4> srcRe_rgb_Im_r : register(SRV_PP_0);
Texture2D<float2> srcIm_gb       : register(SRV_PP_1);
RWTexture2D<float4> dstSignal    : register(UAV_PP_0);
[numthreads(WORKGROUP_SIZE_X, 1, 1)]
void stockhamSharedC2RFFT(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{

	Complex threadBuffer[NUM_CHANNELS][RADIX];
}

#endif
#if 1

    #ifdef STOCKHAM_C2C_INPLACE

RWTexture2D<float4> inoutTexture0 : register(UAV_PP_0);
RWTexture2D<float2> inoutTexture1 : register(UAV_PP_0);

    #else

Texture2D<float4> srcTexture0     : register(SRV_PP_0);
Texture2D<float2> srcTexture1     : register(SRV_PP_1);
RWTexture2D<float4> inoutTexture0 : register(UAV_PP_0);
RWTexture2D<float2> inoutTexture1 : register(UAV_PP_0);

#endif
[numthreads(WORKGROUP_SIZE_X, 1 ,1)]
void stockhamSharedC2CFFT(uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
	
}

#endif

#if 1

[numthreads(WORKGROUP_SIZE_X, 1, 1)]
void stockhamSharedC2C_Convolute_C2C(uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
	
}
#endif
