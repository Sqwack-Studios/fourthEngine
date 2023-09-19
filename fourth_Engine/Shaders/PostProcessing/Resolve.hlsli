float3 acesHdr2Ldr(float3 hdr)
{
	float3x3 m1 = float3x3(
		float3(0.59719f, 0.07600f, 0.02840f),
		float3(0.35458f, 0.90834f, 0.13383f),
		float3(0.04823f, 0.01566f, 0.83777f)
		);
	float3x3 m2 = float3x3(
		float3(1.60475f, -0.10208, -0.00327f),
		float3(-0.53108f, 1.10813, -0.07276f),
		float3(-0.07367f, -0.00605, 1.07602f)
		);

	float3 v = mul(hdr, m1);
	float3 a = v * (v + float3(0.0245786f, 0.0245786f, 0.0245786f)) - float3(0.000090537f, 0.000090537f, 0.000090537f);
	float3 b = v * (float3(0.983729f, 0.983729f, 0.983729f) * v + float3(0.4329510f, 0.4329510f, 0.4329510f)) + float3(0.238081f, 0.238081f, 0.238081f);
	float3 ldr = saturate(mul(a / b, m2));

	return ldr;
}


float computeExposureCompensation(float EV100)
{
	float LMax = (78.0f / (0.65f * 100.0f)) * pow(2.0f, EV100);

	return LMax;
}

float3 adjustExposure(float3 color, float EV100)
{
	float LMax = (78.0f / (0.65f * 100.0f)) * pow(2.0f, EV100);
	return color / LMax;
}

float3 correctGamma(float3 color, float gamma)
{
	return pow(color, 1.0f / gamma);
}

//source: https://google.github.io/filament/Filament.md.html#materialsystem 8.5.1.1
static const float3 validationColors[16] =
{
	{float3(0.0, 0.0, 0.0)},         // black
	{float3(0.0, 0.0, 0.1647)},      // darkest blue
	{float3(0.0, 0.0, 0.3647)},      // darker blue
	{float3(0.0, 0.0, 0.6647)},      // dark blue
	{float3(0.0, 0.0, 0.9647)},      // blue
	{float3(0.0, 0.9255, 0.9255)},   // cyan
	{float3(0.0, 0.5647, 0.0)},      // dark green
	{float3(0.0, 0.7843, 0.0)},      // green
	{float3(1.0, 1.0, 0.0)},         // yellow
	{float3(0.90588, 0.75294, 0.0)}, // yellow-orange
	{float3(1.0, 0.5647, 0.0)},      // orange
	{float3(1.0, 0.0, 0.0)},         // bright red
	{float3(0.8392, 0.0, 0.0)},      // red
	{float3(1.0, 0.0, 1.0)},         // magenta
	{float3(0.6, 0.3333, 0.7882)},   // purple
	{float3(1.0, 1.0, 1.0)}          // white
};

float brightness(in float3 luminance)
{
	return (0.2126 * luminance.r + 0.7152 * luminance.g + 0.0722 * luminance.b);
}
float3 luminanceValidation(in float3 luminance)
{
	// The 5th color in the array (cyan) represents middle gray (18%). Middle gray is .18 in linear scale because of OETF
    // Every stop above or below middle gray causes a color shift
	float v = log2(brightness(luminance) / 0.18);
	v = clamp(v + 5.0, 0.0, 15.0);
	uint index = uint(floor(v));
	return lerp(validationColors[index], validationColors[min(15, index + 1)], frac(v));
}