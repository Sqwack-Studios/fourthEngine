#ifndef _RESOLVE_DEPTH_MS_HLSL
#define _RESOLVE_DEPTH_MS_HLSL

//THIS SHADER NEEDS 
//------------------------------------
//
//Texture2DMS<float> g_depthBuffer : register(srv_RESOLVE_DEPTH);
// 
//-------------------------------------
static const uint MAX_MSAA = 8;

float resolveDepth(float x, float y)
{
	float result = 0.0f;
	uint2 dimensions;
	uint  multisamples;

	g_depthBuffer.GetDimensions(dimensions.x, dimensions.y, multisamples);

	[unroll(MAX_MSAA)]
	for (uint i = 0; i < multisamples; ++i)
	{
		float sampledDepth = g_depthBuffer.Load(float2(x, y), i).r;
		result = max(sampledDepth, result);
	}

	return result;
}




#endif //_RESOLVE_DEPTH_MS_HLSL