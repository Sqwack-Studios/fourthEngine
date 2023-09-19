#include "../Common.hlsli"

#ifndef _FFTs_HLSLI_
#define _FFTs_HLSLI_

float2 complexMul(in float2 z1, in float2 z2)
{
	return float2(z1.x * z2.x - z1.y * z2.y, z1.x * z2.y + z2.x * z1.y);
}

float2 complexConjugate(in float2 z)
{
	return float2(z.x, -z.y);
}

float2 W_N_k(uint N, uint k)
{
	float theta = 2 * _PI * float(k) / float(N);
	float sin, cos;

	sincos(theta, sin, cos);

	return float2(cos, -sin);
}

uint reverseBits32(uint bits)
{
	bits = (bits << 16) | (bits >> 16);
	bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
	bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
	bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
	bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
	return bits;
}



uint reversLowerNBits(uint bits, uint N)
{
	return reverseBits32(bits) >> (32 - N);
}

static const uint WORKGROUP_SIZE_X = 512;
groupshared float2 groupSharedBuffer[WORKGROUP_SIZE_X];

float2 CooleyTukeyFFT(in float2 f_n, in uint index, in float base2TxSize, in uint txSize, in bool bIsForward)
{
	uint reverseIndex = reversLowerNBits(index, base2TxSize);

	GroupMemoryBarrierWithGroupSync();

	groupSharedBuffer[reverseIndex] = f_n;

	for (uint N = 2; N <= txSize; N *= 2)
	{
		uint i = index % N;
		uint k = index % (N / 2);
		uint evenStartIndex = index - i;
		uint oddStartIndex = evenStartIndex + N / 2;

		GroupMemoryBarrierWithGroupSync();

		float2 F_even_k = groupSharedBuffer[evenStartIndex + k];
		float2 F_odd_k = groupSharedBuffer[oddStartIndex + k];

		float2 W = W_N_k(N, k);
		if (!bIsForward) W = complexConjugate(W);

		float2 F_k;
		if (i < N / 2)
			F_k = F_even_k + complexMul(W, F_odd_k);
		else
			F_k = F_even_k - complexMul(W, F_odd_k);

		GroupMemoryBarrierWithGroupSync();
		groupSharedBuffer[index] = F_k;
	}

	GroupMemoryBarrierWithGroupSync();
	float2 F_k = groupSharedBuffer[index];

	return F_k / float(sqrt(txSize));
}

//https://www.intel.com/content/dam/develop/external/us/en/documents/fast-fourier-transform-for-image-processing-in-directx-11-541444.pdf
//REMEMBER TO DEFINE OR STATIC CONST:
//BUTTERFLY_COUNT and TEXTURE_SIZE
//BUTTERFLY_COUNT is log2(TEXTURE_SIZE)
//groupshared float3 pingPongArray[4][TEXTURE_SIZE];

//static const uint BUTTERFLY_COUNT = 9;
//
//void getButterflyValues(in uint passIndex, in uint x, out int2 indices, out float2 weights)
//{
//	int sectionWidth = 2 << passIndex;
//	int halfSectionWidth = sectionWidth / 2;
//
//	int sectionStartOffset = x & ~(sectionWidth - 1);
//	int halfSectionOffset  = x & ~(halfSectionWidth - 1);
//	int sectionOffset = x & (sectionWidth - 1);
//
//	sincos(2.0f * _PI * sectionOffset / (float)sectionWidth, weights.y, weights.x);
//
//	weights.y = -weights.y;
//
//	indices.x = sectionStartOffset + halfSectionOffset;
//	indices.y = sectionStartOffset + halfSectionOffset + halfSectionWidth;
//
//	if (!passIndex)
//	{
//		indices = reversebits(indices) >> (32 - BUTTERFLY_COUNT) & (TEXTURE_SIZE);
//	}
//}

//void butterflyPass(in int passIndex, in uint x, in uint t0, in uint t1, out float3 resultR, out float3 resultI)
//{
//	uint2 indices;
//	float2 weights;
//
//	getButterflyValues(passIndex, x, indices, weights);
//
//	float3 inputR1 = pingPongArray[t0][indices.x];
//	float3 inputI1 = pingPongArray[t1][indices.x];
//
//	float3 inputR2 = pingPongArray[t0][indices.y];
//	float3 inputI2 = pingPongArray[t1][indices.y];
//
//
//	resultR = inputR1 + weights.x * inputR2 - weights.y * inputI2;
//	resultI = inputI1 + weights.y * inputR2 + weights.x * inputI2;
//}


#endif //_FFTs_HLSLI