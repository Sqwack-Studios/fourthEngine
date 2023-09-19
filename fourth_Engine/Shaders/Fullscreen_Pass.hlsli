#ifndef _FULLSCREEN_PASS_HLSLI
#define _FULLSCREEN_PASS_HLSLI
struct Vertex
{
	float3 position;
	float2 uv;
};

//CW
static const Vertex cwVertices[3] =
{ {float3(-1.0f,  3.0f, 1.0f), float2(0.0f, -1.0f)},
  {float3(3.0f,  -1.0f, 1.0f), float2(2.0f, 1.0f)},
  {float3(-1.0f, -1.0f, 1.0f), float2(0.0f, 1.0f)}
};

//CCW
static const Vertex ccwVertices[3] =
{ {float3(-1.0f, -1.0f, 1.0f), float2(0.0f, 1.0f)},
  {float3( 3.0f, -1.0f, 1.0f), float2(2.0f, 1.0f)},
  {float3(-1.0f,  3.0f, 1.0f), float2(0.0f, -1.0f)} };


//WORLD SPACE!
static const Vertex testBILLBOARD[4] =
{ {float3( 0.5f, -0.5f, 0.0f), float2( 0.0f,  1.0f)},
  {float3( 0.5f,  0.5f, 0.0f), float2( 0.0f,  0.0f)},
  {float3(-0.5f,  0.5f, 0.0f), float2( 1.0f,  0.0f)},
  {float3(-0.5f, -0.5f, 0.0f), float2( 1.0f,  1.0f)} };

static const Vertex BILLBOARD[4] =
{ {float3(-0.5f, -0.5f, 0.0f), float2(0.0f,  1.0f)},
  {float3(-0.5f,  0.5f, 0.0f), float2(0.0f,  0.0f)},
  {float3(0.5f,  0.5f, 0.0f), float2(1.0f,  0.0f)},
  {float3(0.5f, -0.5f, 0.0f), float2(1.0f,  1.0f)} };
#endif //_FULLSCREEN_PASS_HLSLI