#include "cbPerView.hlsli"



struct GSOutput
{
	float4 pos : SV_POSITION;
	float linePos : LINEPOS;
};


//2 vertex per line per vertex in a triangle
// 2 * 3
[maxvertexcount(6)]
void main(
	triangle GSInput input[3],
	inout LineStream< GSOutput > output
)
{
	GSOutput element;
	for (uint i = 0; i < 3; i++)
	{

		float4 lineBase = mul(float4(input[i].posWorld, 1.0f), g_VP);
		float4 lineEnd = mul(float4(input[i].posWorld + 0.05f * input[i].normalWorld, 1.0f), g_VP);

		element.linePos = 0.0f;
		element.pos = lineBase;
		output.Append(element);

		element.pos = lineEnd;
		element.linePos = 1.0f;
		output.Append(element);

		output.RestartStrip();
	}
}