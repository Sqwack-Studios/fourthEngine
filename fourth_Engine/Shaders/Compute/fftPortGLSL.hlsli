#ifndef _FFT_PORT_GLSL_HLSLI
#define _FFT_PORT_GLSL_HLSLI

//https://github.com/bane9/OpenGLFFT/blob/main/OpenGLFFT/FFT2D.comp

float2 complexConjugate(in float2 z)
{
	return float2(z.x, -z.y);
}

uint rev_bits(in uint num)
{
	uint count = 31;
	uint reverse_num = num;

	num >>= 1;
	while (num != 0)
	{
		reverse_num <<= 1;
		reverse_num |= num & 1;
		num >>= 1;
		count--;
	}
	reverse_num <<= count;
	return reverse_num;
}

uint index_map(uint threadID, uint currentIteration, uint N)
{
	return ((threadID & (N - (1u << currentIteration))) << 1) | (threadID & ((1u << currentIteration) - 1));
}

uint twiddle_map(uint threadID, uint currentIteration, uint logTwo, uint N)
{
	return (threadID & (N / (1u << (logTwo - currentIteration)) - 1)) * (1u << (logTwo - currentIteration)) >> 1;
}

float2 twiddle(in float q, in bool inverse, in float N)
{
	float theta = float(int(!inverse) * 2 - 1) * 2.0 * _PI * q / N;

	float r = cos(theta);
	float i = sqrt(1.0 - r * r) * float(int(theta < 0.0) * 2 - 1);

	return float2(r, i);
}

static const uint SHARED_BUFFER_SIZE = 1024;
static const uint WORKGROUP_SIZE_X = 64;
static const uint PIXEL_BUFFER_SIZE = 32;

groupshared float real_cache[SHARED_BUFFER_SIZE];
groupshared float imag_cache[SHARED_BUFFER_SIZE];
static float3 pixel_buffer_real[PIXEL_BUFFER_SIZE];
static float3 pixel_buffer_imag[PIXEL_BUFFER_SIZE];

cbuffer fftImageDsc : register(CB_PP_0)
{
	int g_input_width;
	int g_input_height;
	int g_output_width;
	int g_output_height;
	int g_logtwo_width;
	int g_logtwo_height;
	int g_clz_width;
	int g_clz_height;
	int g_no_of_channels;
	int g_stage;
}

void fft_radix2(in int logTwo, in int btid, in int g_offset, in bool is_inverse, in float N)
{
	for (int i = 0; i < logTwo; i++)
	{
		for (int j = btid; j < btid + g_offset; j++)
		{
			uint even = index_map(uint(j), uint(i), uint(N));
			uint odd = even + (1u << uint(i));

			float2 evenVal = float2(real_cache[even], imag_cache[even]);

			uint q = twiddle_map(uint(j), uint(i), uint(logTwo), uint(N));

			float2 e = complexMul(twiddle(float(q), is_inverse, N), float2(real_cache[odd], imag_cache[odd]));

			float2 calculatedEven = evenVal + e;
			float2 calculatedOdd = evenVal - e;

			real_cache[even] = calculatedEven.x;
			imag_cache[even] = calculatedEven.y;

			real_cache[odd] = calculatedOdd.x;
			imag_cache[odd] = calculatedOdd.y;
		}
		GroupMemoryBarrierWithGroupSync();
	}
}


void load_stage0(in int btid, in int g_offset, in int scanline, in int clz_width, in RWTexture2D<float4> input)
{
	for (int i = btid * 2; i < btid * 2 + g_offset * 2; i++)
	{
		int j = int(rev_bits(i) >> clz_width);

		float3 image = input[uint2(j, scanline)].rgb;
		pixel_buffer_real[i - btid * 2] = image;

		pixel_buffer_imag[i - btid * 2] = float3(0.0f, 0.0f, 0.0f);
	}
}

void store_stage0(in int btid, in int g_offset, in int scanline, in RWTexture2D<float4> realXYZimagX, in RWTexture2D<float2> imagYZ)
{
	for (int i = btid * 2; i < btid * 2 + g_offset * 2; i++)
	{
		uint2 idx = uint2(i, scanline);

		float3 real = pixel_buffer_real[i - btid * 2];
		float3 imag = pixel_buffer_imag[i - btid * 2];

		float4 buffer1Color = float4(real, imag.x);
		float2 buffer2Color = imag.yz;

		realXYZimagX[idx] = buffer1Color;
		imagYZ[idx] = buffer2Color;
	}
}

void load_stage1_2(in int btid, in int g_offset, in int scanline, in int clz_height, in RWTexture2D<float4> realXYZimagX, in RWTexture2D<float2> imagYZ)
{
	for (int i = btid * 2; i < btid * 2 + g_offset * 2; i++)
	{
		int j = int(rev_bits(i) >> clz_height);

		float4 buffer1Color = realXYZimagX[uint2(scanline, j)];
		float2 buffer2Color = imagYZ[uint2(scanline, j)];

		float3 real = buffer1Color.xyz;
		float3 imag = float3(buffer1Color.z, buffer2Color);

		pixel_buffer_real[i - btid * 2] = real;
		pixel_buffer_imag[i - btid * 2] = imag;
	}
}

void store_stage1_2(in int btid, in int g_offset, in int scanline, in float N, in RWTexture2D<float4> realXYZimagX, in RWTexture2D<float2> imagYZ)
{
	for (int i = btid * 2; i < btid * 2 + g_offset * 2; i++)
	{
		uint2 idx = uint2(scanline, i);

		float3 colr = pixel_buffer_real[i - btid * 2] * N;
		float3 coli = pixel_buffer_imag[i - btid * 2] * N;

		float4 buffer1Col = float4(colr, coli.x);
		float2 buffer2Col = coli.yz;

		realXYZimagX[idx] = buffer1Col;
		imagYZ[idx] = buffer2Col;
	}
}

void load_stage3(in int btid, in int g_offset, in int clz_width, in int scanline, in RWTexture2D<float4> realXYZimagX, in RWTexture2D<float2> imagYZ)
{
	for (int i = btid * 2; i < btid * 2 + g_offset * 2; i++)
	{
		int j = int(rev_bits(i) >> clz_width);

		float4 buffer1Color = realXYZimagX[uint2(scanline, j)];
		float2 buffer2Color = imagYZ[uint2(scanline, j)];

		float3 real = buffer1Color.xyz;
		float3 imag = float3(buffer1Color.z, buffer2Color);

		pixel_buffer_real[i - btid * 2] = real;

		pixel_buffer_imag[i - btid * 2] = imag;
	}
}

void store_stage3(in int btid, in int g_offset, in int scanline, in int width, in float N, in RWTexture2D<float4> target)
{
	for (int i = btid * 2; i < btid * 2 + g_offset * 2; i++)
	{
		if (i >= width) return;

		float3 col = pixel_buffer_real[i - btid * 2] * N;

		target[uint2(i, scanline)] = float4(col, 0.0f);
		//imageStore(inputImage, ivec2(i, scanline), col);
	}
}

void load_into_cache(in int btid, in int g_offset, in int channel)
{
	for (int i = btid * 2; i < btid * 2 + g_offset * 2; i++)
	{
		real_cache[i] = pixel_buffer_real[i - btid * 2][channel];
		imag_cache[i] = pixel_buffer_imag[i - btid * 2][channel];
	}
}

void load_from_cache(in int btid, in int g_offset, in int channel)
{
	for (int i = btid * 2; i < btid * 2 + g_offset * 2; i++)
	{
		float realCache = real_cache[i];
		pixel_buffer_real[i - btid * 2][channel] = realCache;
		pixel_buffer_imag[i - btid * 2][channel] = imag_cache[i];
	}
}

#endif //_FFT_PORT_GLSL_HLSLI