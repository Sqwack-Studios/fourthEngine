#include "../Fullscreen_Pass.hlsli"

float4 main(uint id : SV_VERTEXID) : SV_POSITION
{

	return float4(cwVertices[id].position, 1.0f);
}