#include "../Fullscreen_Pass.hlsli"

struct VSOut
{
	float4 vertexClip : SV_POSITION;
	float2 uv : UV;
};

VSOut main(uint id: SV_VertexID)
{
	VSOut vout;

	Vertex v = ccwVertices[id];

    vout.vertexClip = float4(v.position, 1.0f);
    vout.uv = v.uv;

    return vout;
}