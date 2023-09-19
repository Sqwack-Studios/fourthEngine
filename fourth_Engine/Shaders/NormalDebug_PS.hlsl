struct PSInput
{
	float4 posClip : SV_POSITION;
    float linePos : LINEPOS;
};

float4 main(PSInput input) : SV_TARGET
{
	float4 outColor = float4(input.linePos, 0.33f + input.linePos * 0.66f, 1.0f, 1.0f);

	return outColor;
}