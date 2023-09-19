#include "../cbPerView.hlsli"

struct VS_CONTROL_POINT_OUTPUT
{
	float3 color : COLOR;
	float globalScale : SCALE;
	float3 posLocal : LPOSITION;
	float3 posWorld : WPOSITION;
	float3 normalWorld : WNORMAL;
};

struct HS_CONTROL_POINT_OUTPUT
{
	float3 color : COLOR;
	float globalScale : SCALE;
	float3 posLocal : LPOSITION;
	float3 posWorld : WPOSITION;
	float3 normalWorld : WNORMAL;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor;
	float InsideTessFactor			: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

float calculateTesselationFactor(float3 p0, float3 p1)
{

#ifdef CAMERA_CENTER_WS
	float3 cameraTop0 = normalize(p0);
	float3 cameraTop1 = normalize(p1);
#else
	float3 cameraPos = g_invV[3].xyz;
	float3 cameraTop0 = normalize(p0 - cameraPos);
	float3 cameraTop1 = normalize(p1 - cameraPos);
#endif


	//Basically, I calculate the angular size of the object. The closer we are, or the bigger the object is in the world, the higher this number is.
	//Calculation is made per side of a triangle.
	//           /----------------- V1 *
	//         -                      / \
	//  Camera/  Alpha               /   \
	//        \                     /     \
	//         -                   /       \
	//           \------------- V0*- - - - -*V2
	//
	// Per side:
	// 1-Calculate camera to vertex vectors
	// 2-Calculate dot product to get cos(apparentSize) 
	// 3-Use cosine to calculate a TF linear function that is adjusted like this:
	// TF(cos(5º)) = 1
	// TF(cos(20º)) = 4     
	//  which yields to :
	// 
	// TF(cos(x)) = m * x + n; 
	// m = -53.095393, n = 53.893349   

	float cosApparentSize = dot(cameraTop0, cameraTop1);

	return clamp(( - 53.095393f * cosApparentSize + 53.893349f), 1.0f, 4.0f);
	

}

HS_CONSTANT_DATA_OUTPUT HologramHSConstantPatch(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	float accumulateTF = 0.0f;
	float TF;
	TF = calculateTesselationFactor(ip[0].posWorld, ip[1].posWorld);
	accumulateTF += TF;					  
	Output.EdgeTessFactor[0] = TF; 
										  
	TF = calculateTesselationFactor(ip[0].posWorld, ip[2].posWorld);
	accumulateTF += TF;					  
	Output.EdgeTessFactor[1] = TF; 
										  
	TF = calculateTesselationFactor(ip[1].posWorld, ip[2].posWorld);
	accumulateTF += TF;
	Output.EdgeTessFactor[2] = TF;

	Output.InsideTessFactor = accumulateTF * 0.33f;

	return Output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HologramHSConstantPatch")]
[maxtessfactor(4.0f)]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> input, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	Output.color = input[i].color;
	Output.globalScale = input[i].globalScale;
	Output.posLocal = input[i].posLocal;
	Output.posWorld = input[i].posWorld;
	Output.normalWorld = input[i].normalWorld;

	return Output;
}
