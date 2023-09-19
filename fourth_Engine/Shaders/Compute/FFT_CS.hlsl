
#include "FFTs.hlsli"

RWTexture2D<float4> input : register(UAV_PP_COMPUTE_0);

RWTexture2D<float4> realRGBimagA : register(UAV_PP_COMPUTE_1);
RWTexture2D<float2> imagRG : register(UAV_PP_COMPUTE_2);


//static const uint FLAG_HORIZONTAL = 1;
//static const uint MODE_FORWARD = 2;
//static const uint MODE_CONVOLUTE = 4;
//static const uint MODE_INVERSE = 8;
//

cbuffer fftImageDsc : register(CB_PP_0)
{
	 uint2 g_fft_size;
	 float g_log2_fft_size;
	 uint g_isForward;
}

[numthreads(WORKGROUP_SIZE_X, 1, 1)]
void main(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
	uint threadIndex = tid.x;
	uint groupIndex = gid.x;
	uint2 uv = uint2(threadIndex, groupIndex);
	float3 src = input[uv].rgb;

	float2 input[3] = { float2(src.x, 0.0f), float2(src.y, 0.0f), float2(src.z, 1.0f) };
	float2 output[3] = { float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f) };

	bool isForward = true;

	for (uint channel = 0; channel < 3; ++channel)
	{
		float2 f_n = input[channel];
		float2 F_k = CooleyTukeyFFT(f_n, threadIndex, g_log2_fft_size, g_fft_size.x, g_isForward);
		output[channel] = F_k;
	}

	float4 outputReal_imX = float4(output[0], output[1]);
	float2 imYZ = output[2];

	realRGBimagA[uv] = outputReal_imX;
	imagRG[uv] = imYZ;
}