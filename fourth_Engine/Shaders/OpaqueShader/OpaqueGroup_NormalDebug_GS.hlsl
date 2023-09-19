#include "../cbPerView.hlsli"

struct GSInput
{
	float4 clip             : SV_POSITION;
	float3 pos              : POSITION;
	float3 normal           : NORMAL;
	float3 tangent          : TANGENT;
	float3 bitangent        : BITANGENT;
	float2 uv               : UV;
};

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

#ifdef  CAMERA_CENTER_WS
row_major float3x3 cameraRotation = float3x3(g_V[0].xyz, g_V[1].xyz, g_V[2].xyz);
#endif

	for (uint i = 0; i < 3; i++)
	{
		float4 lineBase;
		float4 lineEnd;
#ifdef ILLUMINATION_VIEW_SPACE
		lineBase = mul(float4(input[i].pos, 1.0f), g_P);
		lineEnd = mul(float4(input[i].pos + 0.05f * input[i].normal, 1.0f), g_P);
#elif  CAMERA_CENTER_WS
		 
		lineBase = mul(float4(mul(input[i].pos, cameraRotation), 1.0f), g_P);
		lineEnd = mul(float4(mul(input[i].pos + 0.05f * input[i].normal, cameraRotation), 1.0f), g_P);
#else
		lineBase = mul(float4(input[i].pos, 1.0f), g_VP);
		lineEnd = mul(float4(input[i].pos + 0.05f * input[i].normal, 1.0f), g_VP);
#endif
		element.linePos = 0.0f;
		element.pos = lineBase;
		output.Append(element);

		element.pos = lineEnd;
		element.linePos = 1.0f;
		output.Append(element);

		output.RestartStrip();
	}
}