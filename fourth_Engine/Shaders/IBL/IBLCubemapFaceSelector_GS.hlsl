struct VSOut
{
	float4 vertexClip : SV_POSITION;
	float2 uv : UV;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : UV;
	uint slice : SV_RenderTargetArrayIndex;
};

static const uint NUM_FACES = 6;
[maxvertexcount(3 * NUM_FACES)]
void main(
	triangle VSOut input[3], 
	inout TriangleStream< GSOutput > output
)
{

	for (uint face = 0; face < 6; ++face)
	{
		GSOutput vertex;
		vertex.slice = face;

		for (uint v = 0; v < 3; ++v)
		{
			vertex.pos = input[v].vertexClip;
			vertex.uv = input[v].uv;

			output.Append(vertex);
		}
		output.RestartStrip();
	}

}