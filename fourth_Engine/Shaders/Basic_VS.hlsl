
struct VSOutput
{
	float4 posCS : SV_POSITION;
	float4 color : COLOR;
};

VSOutput main(float3 pos : POSITION, float4 color : COLOR)
{
	VSOutput Output;

	Output.posCS = float4(pos.x, pos.y, pos.z, 1.0f);
	Output.color = color;

	return Output;
}
