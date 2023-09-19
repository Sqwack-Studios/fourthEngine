#include "../cbPerView.hlsli"


//SOURCE : http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/

struct PSIn
{
	float4 posClip : SV_POSITION;
    float3 nearPoint : NPOINT;
    float3 farPoint : FPOINT;
};

struct PSOut
{
	float4 color : SV_TARGET;
	float  depth : SV_Depth;
};

float4 grid(float3 fragWorldPos, float scale, float lineWidth, bool tintXZ)
{
	float2 coord = fragWorldPos.xz * scale;
	float2 derivative = fwidth(coord);
	float2 grid = abs(frac(coord - 0.5f) - 0.5f) / derivative ;
	float lline = min(grid.x, grid.y) - lineWidth;
	float minimumz = min(derivative.y, 1.f);
	float minimumx = min(derivative.x, 1.f);

	float4 color;
	color.rgb = float3(0.2f, 0.2f, 0.2f);
	color.a = 1.0 - min(lline, 1.0);

	if (fragWorldPos.x > -minimumx && fragWorldPos.x < minimumx)
	{
		color.rgb = float3(1.0f, 0.0f, 0.0f) * tintXZ;
	}
	else if (fragWorldPos.z > -minimumz && fragWorldPos.z < minimumz)
	{
		color.rgb = float3(0.0f, 0.0f, 1.0f) * tintXZ;
	}


	return color;
}

float4 computeClipSpace(float3 fragWorldPos, float4x4 vpMatrix)
{
	return mul(float4(fragWorldPos, 1.0f), vpMatrix);
}

float computeDepth(float4 posClip)
{
	return posClip.z / posClip.w;
}
float computeLinearDepth(float depth)
{
	float remapDepth = depth * 2.0f - 1.0f;
	float linearDepth = (2.0 * g_nearPlane * g_farPlane) / (g_farPlane + g_nearPlane - remapDepth * (g_nearPlane - g_farPlane));

	return linearDepth / g_farPlane;
}

PSOut main(PSIn input) 
{
	float4x4 worldToClip = float4x4(g_VP);

	float1 t = -input.nearPoint.y / (input.farPoint.y - input.nearPoint.y);
	float3 posWorld = input.nearPoint + t * (input.farPoint - input.nearPoint);


	float4 posClip = computeClipSpace(posWorld, worldToClip);
	float depth = computeDepth(posClip);
	float fade = max(0.f, (0.4 - computeLinearDepth(depth)));


	PSOut psout;
	float4 gridColor = (grid(posWorld, .1f, .3f, false) + grid(posWorld, 1.0f, 0.1f, true) ) * (t > 0);
	float alpha = gridColor.a;
	gridColor.a *= fade;


	psout.depth = posClip.z / posClip.w * (alpha);
	psout.color = gridColor;

	return psout;
}