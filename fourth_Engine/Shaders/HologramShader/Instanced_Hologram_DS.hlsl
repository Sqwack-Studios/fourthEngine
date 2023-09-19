struct DS_OUTPUT
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

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	//no interp
	Output.color = patch[0].color;
	Output.globalScale = patch[0].globalScale;

	Output.posLocal = domain.x * patch[0].posLocal + domain.y * patch[1].posLocal + domain.z * patch[2].posLocal;
	Output.posWorld = domain.x * patch[0].posWorld + domain.y * patch[1].posWorld + domain.z * patch[2].posWorld;
	Output.normalWorld = domain.x * patch[0].normalWorld + domain.y * patch[1].normalWorld + domain.z * patch[2].normalWorld;

	return Output;
}
