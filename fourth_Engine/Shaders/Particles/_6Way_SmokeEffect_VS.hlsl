#include "../cbPerView.hlsli"
#include "../Common.hlsli"
#include "../Fullscreen_Pass.hlsli"//contains billboard vertices!
struct InInstance
{
	//Instance parameters only
	float4 color        : COLOR;
	float3 position     : POSITION;
	float  rotation     : ROTATION;
	float3 speed        : SPEED;
	float  timeFraction : TIME;
	float2 size         : SIZE;
	uint   tileIdx      : TILE;
};

struct VSOut
{
	float4 clip                 : SV_POSITION;
    float3 worldPos             : POSITION;
	float2 uv                  : UV;
	nointerpolation float    timeFraction : TIME;
	nointerpolation float3   up  : UP;
	nointerpolation float3   forward : FORWARD;
	nointerpolation uint     tileIdx : TILE;
 	nointerpolation float4 color : COLOR;

};




VSOut main(InInstance instance, uint index : SV_VertexID )
{
	//We sample from a billboard set in world origin with size (1, 1)

	Vertex currentVertex = BILLBOARD[index];

	float sin, cos;
	sincos(instance.rotation, sin, cos);
	

	float3 right, up, forward;

	up = float3(0.0f, 1.0f, 0.0f);

#ifdef CAMERA_CENTER_WS
	forward = -instance.position;
#else
	forward = g_invV[3].xyz - instance.position;
#endif
	
	forward = normalize(forward);
	right = normalize(cross(up, forward));
	up    = normalize(cross(forward, right));

	row_major float2x2 rotateOffsets = { float2(cos, -sin), float2(sin, cos) };

	float3 vertexPos;
	float4 clip;

#ifdef CAMERA_CENTER_WS
	row_major float3x3 cameraRotation = float3x3(g_V[0].xyz, g_V[1].xyz, g_V[2].xyz);

	vertexPos = mul(instance.position, cameraRotation).xyz;
	vertexPos.xy += instance.size * mul(currentVertex.position.xy, rotateOffsets);
	clip = mul(float4(vertexPos, 1.0f), g_P);

	vertexPos = mul(vertexPos, transpose(cameraRotation));

#else
	vertexPos = mul(float4(instance.position, 1.0f), g_V).xyz;
	vertexPos.xy += instance.size * mul(currentVertex.position.xy, rotateOffsets);
	clip = mul(float4(vertexPos, 1.0f), g_P);

	vertexPos = mul(float4(vertexPos, 1.0f), g_invV).xyz;

#endif



	VSOut vout;

	vout.clip = clip;
	vout.worldPos = vertexPos;
	vout.uv = currentVertex.uv;
	vout.timeFraction = instance.timeFraction;
	vout.up = right;
	vout.forward = forward;
	vout.tileIdx = instance.tileIdx;
	vout.color = instance.color;
	return vout;
}