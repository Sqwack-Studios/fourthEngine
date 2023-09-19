#include "HologramEffect.hlsli"

struct GSInput
{
	float3 color : COLOR;
	float globalScale : SCALE;
	float3 posLocal : LPOSITION;
	float3 posWorld : WPOSITION;
	float3 normalWorld : WNORMAL;
};

struct GSOutput
{
	float4 posClip: SV_POSITION;
	float3 color: COLOR;
	float3 posLocal: LPOSITION;
	float3 posWorld: WPOSITION;
	float3 normalWorld: WNORMAL;
};


[maxvertexcount(3)]

void main(
	triangle GSInput input[3], 
	inout TriangleStream< GSOutput > output
)
{
	float4x4 worldToClip = float4x4(g_VP);

	//Calculate triangle normal using vertex points
	//If we distort per vertex, faces won't break from the model. Adjacent faces will bend,
	//following sinusoidal shape. 
	//To break the triangles from the model, we have to distort vertices equally. In order to do that,
	//we calculate triangle centroid and distort that point (distort basically means sample the height of the wave)
	//based on triangle normal. Then, we offset all 3 vertices by the distortion factor along its normal

	float3 P1P0 = input[1].posWorld - input[0].posWorld;
	float3 P2P0 = input[2].posWorld - input[0].posWorld;
	float3 triangleNormal = normalize(cross(P1P0, P2P0));
	float3 triangleCentroid = (input[0].posLocal + input[1].posLocal + input[2].posLocal) * 0.3333f;

	float distortion = input[0].globalScale * distortPoint(triangleCentroid);

#ifdef CAMERA_CENTER_WS
	row_major float3x3 cameraRotation = float3x3(g_V[0].xyz, g_V[1].xyz, g_V[2].xyz);
#endif

	GSOutput element;

	[unroll]
	for (uint i = 0; i < 3; ++i)
	{
		float3 posWorldDistorted = input[i].posWorld + distortion * triangleNormal;

#ifdef CAMERA_CENTER_WS
		element.posClip = mul(float4(mul(posWorldDistorted, cameraRotation), 1.0f), g_P);
#else
		element.posClip = mul(float4(posWorldDistorted, 1.0f), worldToClip);
#endif
		element.posLocal = input[i].posLocal;
		element.posWorld = posWorldDistorted;
		element.normalWorld = input[i].normalWorld;
		element.color = input[i].color;

		output.Append(element);
	}


}