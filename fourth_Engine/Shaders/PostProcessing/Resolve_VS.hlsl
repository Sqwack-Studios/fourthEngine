#include "../Fullscreen_Pass.hlsli"

struct VSOut
{
	float4 pos : SV_POSITION;
};

VSOut main(uint id: SV_VertexID)
{
	VSOut vout;

	vout.pos = float4(cwVertices[id].position.xy, 1.0f, 1.0f);
	return vout;
}