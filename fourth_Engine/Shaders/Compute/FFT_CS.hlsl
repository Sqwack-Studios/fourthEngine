

//RWTexture2D<float4> input : register(UAV_PP_COMPUTE_0);

//RWTexture2D<float4> realRGBimagA : register(UAV_PP_COMPUTE_1);
//RWTexture2D<float2> imagRG : register(UAV_PP_COMPUTE_2);


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

[numthreads(WORKGROUP_SIZE_X, 1, 1)]
void main(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{

	Complex threadBuffer[4][RADIX];
}