//https://zhuanlan.zhihu.com/p/611582936 I have no idea about implementing FFT but it works
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


//cbuffer fftImageDsc : register(CB_PP_0)
//{
//	uint2 g_fft_size;
//	float g_log2_fft_size;
//	uint g_isForward;
//}
//
//[numthreads(WORKGROUP_SIZE_X, 1, 1)]
//void main(uint3 id : SV_DispatchThreadID, uint3 tid : SV_GroupThreadID, uint3 gid : SV_GroupID)
//{
//	uint threadIndex = tid.x;
//	uint groupIndex = gid.x;
//	uint2 uv = uint2(threadIndex, groupIndex);
//	float3 src = input[uv].rgb;
//
//	float2 input[3] = { float2(src.x, 0.0f), float2(src.y, 0.0f), float2(src.z, 1.0f) };
//	float2 output[3] = { float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f) };
//
//	bool isForward = true;
//
//	for (uint channel = 0; channel < 3; ++channel)
//	{
//		float2 f_n = input[channel];
//		float2 F_k = CooleyTukeyFFT(f_n, threadIndex, g_log2_fft_size, g_fft_size.x, g_isForward);
//		output[channel] = F_k;
//	}
//
//	float4 outputReal_imX = float4(output[0], output[1]);
//	float2 imYZ = output[2];
//
//	realRGBimagA[uv] = outputReal_imX;
//	imagRG[uv] = imYZ;
//}