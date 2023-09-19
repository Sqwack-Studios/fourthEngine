#include "../cbPerView.hlsli"
#include "../RenderTargets.hlsli"

struct PSIn
{
	float4 posClip        : SV_POSITION;
	float3 posWorld       : WPOSITION;
	float3 normalWorld    : WNORMAL;
	nointerpolation float3 emissionColor    : COLOR;
};


float4 main(PSIn pin) : TARGET_EMISSION
{
    float3 emissionColor = pin.emissionColor;
	float3 normedEmission = emissionColor / max(emissionColor.x, max(emissionColor.y, max(emissionColor, 1.0)));
	float3 cameraDir = normalize(g_invV[3].xyz - pin.posWorld);
	float3 normal = normalize(pin.normalWorld);
	float3 color;
	float NoV = dot(cameraDir, normal);
    float contour; 

//my effect :)

	contour = pow(max(0.0f, NoV), 4.0f);
	color = lerp(normedEmission, emissionColor, contour);

	//emil's effect is darker in the contours
	// 
	//contour = pow(max(0.0, NoV), 8);
	//color = lerp(normedEmission * 0.33, emissionColor, contour);




	return float4(color, 1.0f);
}