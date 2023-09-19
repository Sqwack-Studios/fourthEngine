#include "Resolve.hlsli"

#ifndef _BLOOM_HLSLI_
#define _BLOOM_HLSLI_

//BLOOM OPTIONS THAT I'VE THOUGHT OF:

//1- Apply CoD solution, using a 13 tap bilinear sample during Downsample stage, then upsample using a 3x3 tap filter during upsampling stage. It just requires a single mipchain.
//2- Apply a Kawasa filter, which behaves (kinda) like a gaussian blur.
//3- Downsample using a gaussian filter. Upsample using a simple averaged 2x2 bilinear filter. Requires 2 mipchains, because separable gaussian filter requires first pass output as input for the second pass.
//4- Downsample using CoD 13 tap downsampling stage, but upsample using gaussian filter, which requires up to 3 mipchains. Upsampling requires composition, and we cannot override the downsampling stage mipchain, as we need it 
//in the composition stage: Downsample -> Gaussian stage: vertical, horizontal + composite(takes as input MIP N-1 and previos gaussian output, so we are at 3 mipchains)
//5- I've seen people just making the gaussian downsample and using the mipchain as input into the resolve HDR stage without fireflight and temporal stability issues if you use a large gaussian kernel. No composition and upsampling stage.
//6- An awesome solution, but expensive, would be to apply a FFT to the HDR source, and convolute it with a point spread function as I did when I was studying difractive optics.

static const int MODE_DOWNSAMPLE = 0;
static const int MODE_UPSAMPLE = 1;
static const int FLAG_GAUSSIAN_HORIZONTAL = 1;
static const int FLAG_GAUSSIAN_COMPOSITE = 2; 
//because gaussian dimension separation needs two passes per mip, during upscale stage we need to composite
//prev mip with blurred mip, so we have to sample the downsampled mip and add to the result in the last pass.

//CoD Siggraph 2014 presentation
// Take 13 samples around current texel:
// a - b - c
// - j - k -
// d - e - f
// - l - m -
// g - h - i
// === ('e' is the current texel) ===


float3 downsampleFirstPass(in sampler txSampler, in Texture2D<float4> hdr, in float2 texels, in float2 invResolution)
{
	float3 innerBox = float3(0.0f, 0.0f, 0.0f);
	float3 outerBox = float3(0.0f, 0.0f, 0.0f);
	float3 center = float3(0.0f, 0.0f, 0.0f);


// Apply weighted distribution:
// 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
// a,b,d,e * 0.125
// b,c,e,f * 0.125
// d,e,g,h * 0.125
// e,f,h,i * 0.125
// j,k,l,m * 0.5

	//OuterBox
	outerBox += hdr.SampleLevel(txSampler, (texels + float2(-2.0f,  2.0f)) * invResolution, 0).rgb;
	outerBox += hdr.SampleLevel(txSampler, (texels + float2( 0.0f,  2.0f)) * invResolution, 0).rgb;
	outerBox += hdr.SampleLevel(txSampler, (texels + float2( 2.0f,  2.0f)) * invResolution, 0).rgb;

	outerBox += hdr.SampleLevel(txSampler, (texels + float2(-2.0f,  0.0f)) * invResolution, 0).rgb;
	outerBox += hdr.SampleLevel(txSampler, (texels + float2( 2.0f,  0.0f)) * invResolution, 0).rgb;

	outerBox += hdr.SampleLevel(txSampler, (texels + float2(-2.0f, -2.0f)) * invResolution, 0).rgb;
	outerBox += hdr.SampleLevel(txSampler, (texels + float2( 0.0f, -2.0f)) * invResolution, 0).rgb;
	outerBox += hdr.SampleLevel(txSampler, (texels + float2( 2.0f, -2.0f)) * invResolution, 0).rgb;

	outerBox *= 0.125f;

	//InnerBox
	innerBox += hdr.SampleLevel(txSampler, (texels + float2(-1.0f,  1.0f)) * invResolution, 0).rgb;
	innerBox += hdr.SampleLevel(txSampler, (texels + float2( 1.0f,  1.0f)) * invResolution, 0).rgb;
	innerBox += hdr.SampleLevel(txSampler, (texels + float2(-1.0f, -1.0f)) * invResolution, 0).rgb;
	innerBox += hdr.SampleLevel(txSampler, (texels + float2( 1.0f, -1.0f)) * invResolution, 0).rgb;
	innerBox *= 0.5f;

	//Center
	center = hdr.SampleLevel(txSampler, (texels) * invResolution, 0).rgb;
	center *= 0.125f;



	return (innerBox + outerBox + center) * 0.25f;
}


float3 upsampleBloom(in sampler txSampler, in Texture2D<float4> target, in float2 uv, in float radius, in float2 invResolution)
{
	const float4 offsets = invResolution.xyxy * float4(1.0f, 1.0f, -1.0f, 0.0f) * radius;

// Take 9 samples around current texel:
 // a - b - c
 // d - e - f
 // g - h - i
 // === ('e' is the current texel) ===

// Apply weighted distribution, by using a 3x3 tent filter:
//  1   | 1 2 1 |
// -- * | 2 4 2 |
// 16   | 1 2 1 |

	float3 result = float3(0.0f, 0.0f, 0.0f);
	float3 borders = float3(0.0f, 0.0f, 0.0f);
	float3 sides = float3(0.0f, 0.0f, 0.0f);
	float3 center = float3(0.0f, 0.0f, 0.0f);

	borders += target.SampleLevel(txSampler, uv - offsets.xy, 0).rgb;
	borders += target.SampleLevel(txSampler, uv - offsets.zy, 0).rgb;
	borders += target.SampleLevel(txSampler, uv + offsets.xy, 0).rgb;
	borders += target.SampleLevel(txSampler, uv + offsets.zy, 0).rgb;

	sides += target.SampleLevel(txSampler, uv - offsets.wy, 0).rgb;
	sides += target.SampleLevel(txSampler, uv + offsets.zw, 0).rgb;
	sides += target.SampleLevel(txSampler, uv + offsets.xw, 0).rgb;
	sides += target.SampleLevel(txSampler, uv + offsets.wy, 0).rgb;

	center += target.SampleLevel(txSampler, uv, 0).rgb;

	return (borders + sides * 2.0f + center * 4.0f) * 0.0625f; // 1/16

}


//https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/shaders/blur_gaussian_float4CS.hlsl

static const int GAUSSIAN_KERNEL = 33;
static const int TILE_BORDER = GAUSSIAN_KERNEL / 2;
#define GAUSSIAN_BLUR_THREADCOUNT  128
static const int CACHE_SIZE = TILE_BORDER + GAUSSIAN_BLUR_THREADCOUNT + TILE_BORDER;

static const float gaussianWeightsNormalized[GAUSSIAN_KERNEL] = {
	0.004013f,
	0.005554f,
	0.007527f,
	0.00999f,
	0.012984f,
	0.016524f,
	0.020594f,
	0.025133f,
	0.030036f,
	0.035151f,
	0.040283f,
	0.045207f,
	0.049681f,
	0.053463f,
	0.056341f,
	0.058141f,
	0.058754f,
	0.058141f,
	0.056341f,
	0.053463f,
	0.049681f,
	0.045207f,
	0.040283f,
	0.035151f,
	0.030036f,
	0.025133f,
	0.020594f,
	0.016524f,
	0.012984f,
	0.00999f,
	0.007527f,
	0.005554f,
	0.004013f
};
static const int gaussianOffsets[GAUSSIAN_KERNEL] = {
	-16,
	-15,
	-14,
	-13,
	-12,
	-11,
	-10,
	-9,
	-8,
	-7,
	-6,
	-5,
	-4,
	-3,
	-2,
	-1,
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
};

#endif //_BLOOM_HLSLI_