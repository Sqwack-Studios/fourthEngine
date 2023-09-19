#include "../Fullscreen_Pass.hlsli"
#include "../cbPerView.hlsli"

struct VSOut
{
	float4 vertexClip : SV_POSITION;
	float3 direction : DIRECTION;
};

VSOut main(uint id: SV_VertexID)
{
	VSOut vout;

	
	float3 offset = g_frustumCornersDirections[id].xyz - g_frustumCornersDirections[0].xyz;


	vout.direction = g_frustumCornersDirections[id].xyz + offset;


	vout.vertexClip = float4(ccwVertices[id].position, 1.0f);

	


	return vout;
}