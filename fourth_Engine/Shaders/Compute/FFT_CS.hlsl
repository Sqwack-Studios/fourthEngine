////////////////////////////////////////////////////////////////////////////////////////////

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

//Something that could improve our bandwitch: first, let's note that for a RGB fourier transform, we need 6 * float values that, right now
//we are packing as RGBA + RG independent textures
//our convolution kernel is also a complex RGB signal (even though we could remove color dispersion contribution by using just a single R channel)

//So, we have up to 12 * float values: 6 (HDR signal) + 6 (kernel signal). We can use 3 * RGBA buffers
//tx0 : RGB -> HDR signal Real      part  ......... A -> kernel signal Real X component
//tx1 : RGB -> HDR signal Imaginary part  ......... A -> kernel signal Im   X component
//tx2 : RGBA -> kernel signal Real + Im YZ components.

//This way we could be improving our memory write patterns based on https://www.youtube.com/watch?v=eDLilzy2mq0&t=2570s time stamp: 21:40.
//This is obviously making limitations if we really need to operate in our kernel, as we have to make up to 3 scattered writes. However, we are not supposed to be 
//accessing our kernel every frame except if our resolution changes, unless I'm missing something.

//During convolution stage, we load each texture at the same time, so maybe we can also make any necessary kernel operation in that pass. As I say, this truly is inflexible.
//Using 2-for-1 trick for R2C, C2R FFT will make all of this much faster, and our kernel filter will also utilize a single buffer. In any case, that would make our kernel much most
//costly to compute with, because we have to unpack symmetric data.
//#ifdef FFT_R2C
#if STOCKHAM_R2C

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

#if STOCKHAM_C2R


///////////////////////////////////////////////////////////////////////////////////////////
//Computes a Stockham LDS FFT Complex-To-Real. Performs three FFTs from a source texture.
//
//TODO: We are dropping support to apply a FFT to the alpha channel, as this would generate (x + iy) * 4 channels.
//We would need two full fp32 textures and might hurt bandwitch. I was planning to load the entire FFT in registers in one go.
//By adding another channel, we might get register overuse. I have not made any calculation nor profiling, but I wanna get it working.
//
//SrcTexture0 == srcRe_rgb_Im_r -> a float4 texture that contains RGB: Real part of the signal, A: x component of the imaginary part of the signal
//SrcTexture1 == srcIm_rg       -> a float2 texture that contains RG : The rest of the imaginary part
//DstTexture                    -> a float4 texture that contains RGB real data of the FFT. Alpha channel just writes 0.                  
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////

Texture2D<float4> srcRe_rgb_Im_r : register(SRV_PP_0);
Texture2D<float2> srcIm_gb       : register(SRV_PP_1);
RWTexture2D<float4> dstSignal    : register(UAV_PP_0);
[numthreads(WORKGROUP_SIZE_X, 1, 1)]
void stockhamSharedC2RFFT(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{

	Complex threadBuffer[NUM_CHANNELS][RADIX];
}

#endif

#if STOCKHAM_C2C

///////////////////////////////////////////////////////////////////////////////////////////
//Computes a Stockham LDS FFT Complex-To-Complex. Performs three FFTs from two source textures.
//
//TODO: We are dropping support to apply a FFT to the alpha channel, as this would generate (x + iy) * 4 channels.
//We would need two full fp32 textures and might hurt bandwitch. I was planning to load the entire FFT in registers in one go.
//By adding another channel, we might get register overuse. I have not made any calculation nor profiling, but I wanna get it working.
//
//SrcTexture0  -> a float4 texture that contains RGB: Real part of the signal, A: x component of the imaginary part of the signal
//SrcTexture1  -> a float2 texture that contains RG : The rest of the imaginary part
//
//DstTexture0  -> a float4 texture that contains RGB real data of the FFT. Alpha channel just writes 0.                  
//DstTexture1  -> a float4 texture that contains RGB real data of the FFT. Alpha channel just writes 0.                  
//
//It can be performed inplace, overwriting input data(as long as signal size is small enough to fit in LDS memory)
//
////////////////////////////////////////////////////////////////////////////////////////////

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

#ifdef STOCKHAM_LDS_C2C_CONV_C2C

///////////////////////////////////////////////////////////////////////////////////////////
//Computes a Stockham LDS FFT Complex-To-Complex, convolutes with a kernel, and makes another FFT inverse pass
//This is a bigger shader to reduce dispatch calls and packs a horizontal/vertical forward pass, convolution with a provided kernel, and horizontal/vertical inverse.
//Performs three FFTs per horizontal/vertical pass, and N multiplications with a size N kernel.
//
//TODO: We are dropping support to apply a FFT to the alpha channel, as this would generate (x + iy) * 4 channels.
//We would need two full fp32 textures and might hurt bandwitch. I was planning to load the entire FFT in registers in one go.
//By adding another channel, we might get register overuse. I have not made any calculation nor profiling, but I wanna get it working.
//
//inoutTexture0  -> a float4 texture that contains RGB: Real part of the signal, A: x component of the imaginary part of the signal
//inoutTexture1  -> a float2 texture that contains RG : The rest of the imaginary part
//
//srcTexture0  -> a float4 kernel filter texture that contains RGB real data of the FFT. Alpha channel just writes 0.  
//srcTexture1  -> a float2 kernel filter texture that contains RG : The rest of the imaginary part                 
//
//It can be performed inplace, overwriting input data(as long as signal size is small enough to fit in LDS memory)
//
////////////////////////////////////////////////////////////////////////////////////////////

RWTexture2D<float4> inoutTexture0  : register(UAV_PP_0);
RWTexture2D<float2> inoutTexture0  : register(UAV_PP_0);
Texture2D<float4>   kernelTexture0 : register(SRV_PP_0);
Texture2D<float2>   kernelTexture1 : register(SRV_PP_1);

[numthreads(WORKGROUP_SIZE_X, 1, 1)]
void stockhamSharedC2C_Convolute_C2C(uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
	
}

#endif
