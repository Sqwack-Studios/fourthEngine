#include "../cbPerView.hlsli"

//SOURCE : http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/

//TODO: from lecture #6: Use a single triangle to render the grid?
struct VSOut
{
	float4 posClip : SV_POSITION;
	float3 nearPoint : NPOINT;
	float3 farPoint : FPOINT;
};

static const float3 vertices[6] =
{ float3(1.0f, 1.0f, 1.0f), float3(-1.0f, -1.0f, 1.0f), float3(-1.0f, 1.0f, 1.0f),
  float3(-1.0f, -1.0f, 1.0f), float3(1.0f, 1.0f, 1.0f), float3(1.0f, -1.0f, 1.0f) };


VSOut main(uint id: SV_VertexID)
{
	float4x4 invViewToClip = float4x4(g_invVP);

	VSOut vout;
	vout.posClip = float4(vertices[id], 1.0f);
	float4 nearPoint = mul(float4(float3(vertices[id].xy, 1.0f), 1.0f), invViewToClip);
	float4 farPoint = mul(float4(float3(vertices[id].xy, 0.0f), 1.0f), invViewToClip);

	vout.nearPoint = nearPoint.xyz / nearPoint.w;
	vout.farPoint = farPoint.xyz / farPoint.w;


	return vout;

}