#include "../Registers.hlsli"
#include "../ShadowMapping/cbShadowMap.hlsli"

struct GSOutput
{
	float4 pos : SV_POSITION;
	uint   targetSlice : SV_RenderTargetArrayIndex;
	float2 uv       : UV;
	float2 time     : TIME;
};

struct VSOut
{
	float3 pos      : POSITION;
	float2 uv       : UV;
	float2 time     : TIME;
};
static const uint CUBE_FACES = 6;
static const uint MAX_VERTICES = 3 * CUBE_FACES;

//So, each instance index represent a cubemap face: X, -X, Y, -Y, Z, -Z
//                                                  0,  1, 2,  3, 4,  5
//Also, cb_SHADOW_PASS_CUBE contains g_startIndexTarget, which represents which slice of the TextureCubeArray we are targeting
//Our final target will be g_startIndexTarget  +  InstanceID
//cb_SHADOW_PASS_CUBE contains an array of matrices that matches a cubemap face of a single light position, so use InstanceID to point to correct matrix



[maxvertexcount(MAX_VERTICES)]
[instance(CUBE_FACES)]
void main(
	triangle VSOut input[3], uint InstanceID : SV_GSInstanceID,
	inout TriangleStream< GSOutput > output
)
{
	GSOutput vertex;

	float4x4 VP = g_faceVP[InstanceID];
	uint finalArrayTarget = InstanceID + g_startIndexTarget;

	vertex.targetSlice = finalArrayTarget;
	for (uint v = 0; v < 3; ++v)
	{
		vertex.pos = mul(float4(input[v].pos, 1.0f), VP);
		vertex.uv = input[v].uv;
		vertex.time = input[v].time;
		output.Append(vertex);
	}
}