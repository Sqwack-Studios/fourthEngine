#include "RenderTargets.hlsli"
#include "Registers.hlsli"
#include "sSamplers.hlsli"


#ifndef _COMMON_HLSL
#define _COMMON_HLSL

static const float _PI        = 3.141592654f;
static const float _2PI       = 6.283185307f;
static const float _1DIVPI    = 0.318309886f;
static const float _1DIV2PI   = 0.159154943f;
static const float _1DIV4PI   = 0.079577475f;
static const float _PIDIV2    = 1.570796327f;
static const float _PIDIV4    = 0.785398163f;

static const float  EULER      = 2.718281828f;
static const float  _1DIVEULER = 0.367879441f;

static const float EPSILON    = 0.01f;
// Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
static const float MIN_NoV_CLAMP = 1e-4f;

//Equation obtained from MS documentation: https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
float linearizeDepth(in float currentDepth, in float near, in float far)
{
	float num = near * far;
	float den = far - (1.0f - currentDepth) * (far - near);

	return num / den;
}


//Equation obtained from MS documentation: https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
float computeDepth(in float linearDepth, in float near, in float far)
{
	float inv_far_minus_near = 1.0f / (far - near);

	float sum = far * inv_far_minus_near - far * near * inv_far_minus_near * 1.0f / linearDepth;

	return sum;
}

float3 transformVector(in float3 vec, in row_major float4x4 transform)
{
	return vec.x * normalize(transform[0].xyz) + vec.y * normalize(transform[1].xyz) + vec.z * normalize(transform[2].xyz);
}

float3 transformVector(in float3 vec, in row_major float3x3 rotation)
{
	return vec.x * normalize(rotation[0].xyz) + vec.y * normalize(rotation[1].xyz) + vec.z * normalize(rotation[2].xyz);
}

float3 transformVectorOrthonormal(in float3 vec, in row_major float4x4 transform)
{
	return vec.x * transform[0].xyz + vec.y * transform[1].xyz + vec.z * transform[2].xyz;
}

float3 transformVectorOrthonormal(in float3 vec, in row_major float3x3 rotation)
{
	return vec.x * rotation[0].xyz + vec.y * rotation[1].xyz + vec.z * rotation[2].xyz;
}


float3x3 computeRotationMatrix(in float3 axis, in float sin, in float cos)
{

	float _1minusCos = 1.0f - cos;
	float3 v = axis;
	float _1minusCosXY = _1minusCos * v.x * v.y;
	float _1minusCosXZ = _1minusCos * v.x * v.z;
	float _1minusCosYZ = _1minusCos * v.y * v.z;

	row_major float3x3 rollRotation =
		float3x3 (
			float3(cos + _1minusCos * v.x * v.x, _1minusCosXY + sin * v.z, _1minusCosXZ - sin * v.y),
			float3(_1minusCosXY - sin * v.z, cos + _1minusCos * v.y * v.y, _1minusCosYZ + sin * v.x),
			float3(_1minusCosXZ + sin * v.y, _1minusCosYZ - sin * v.x, cos + _1minusCos * v.z * v.z));

	return rollRotation;
}

float3x3 computeRotatonMatrix(in float3 axis, in float angle, out float outSin, out float outCos)
{
	float sin, cos;
	sincos(angle, sin, cos);

	outSin = sin;
	outCos = cos;

	return computeRotationMatrix(axis, sin, cos);
}

float3 unpackNormalMap(in float3 txSample, in bool invertY, in bool buildBlue)
{
	float3 normalRGB = txSample.rgb;

	if (invertY)
		normalRGB.g = 1.0f - normalRGB.g;

	float3 remap = normalRGB * 2.0f - 1.0f;


	if (buildBlue)
	{
		return float3(remap.rg, sqrt(saturate(1.0f - dot(remap.rg, remap.rg))));
	}

	return remap;

}

float2 nonZeroSign(float2 v)
{
	return float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
}

float2 packOctahedron(float3 v)
{
	float2 p = v.xy / (abs(v.x) + abs(v.y) + abs(v.z));
	return v.z <= 0.0 ? (float2(1.0f, 1.0f) - abs(p.yx)) * nonZeroSign(p) : p;
}

float3 unpackOctahedron(float2 oct)
{
	float3 v = float3(oct, 1.0 - abs(oct.x) - abs(oct.y));
	if (v.z < 0) v.xy = (float2(1.0f, 1.0f) - abs(v.yx)) * nonZeroSign(v.xy);
	return normalize(v);
}

float computeAlphaToCoverage(in float alpha, in float threshold, in float falloff)
{
	float num = alpha - threshold;
	float den = fwidth(alpha) * falloff;

	return saturate(num / den);
}

//Returns normalized direction from UV and face
float3 lookupCubemapFrom_UVFace(in float2 uv, in uint face)
{
	float2 remapUV = uv * 2.0f - 1.0f;

	float3 direction = float3(1.0f, 1.0f, 1.0f);

	if (face == 0)//+X
	{
		direction = float3(1.0f, -remapUV.y, -remapUV.x);
	}
	else if (face == 1)//-X
	{
		direction = float3(-1.0f, -remapUV.y, remapUV.x);
	}
	else if (face == 2)//+Y
	{
		direction = float3(remapUV.x, 1.0f, remapUV.y);
	}
	else if (face == 3)//-Y
	{
		direction = float3(remapUV.x, -1.0f, -remapUV.y);
	}
	else if (face == 4)//+Z
	{
		direction = float3(remapUV.x, -remapUV.y, 1.0f);
	}
	else if (face == 5)//-Z
	{
		direction = float3(-remapUV.x, -remapUV.y, -1.0f);
	}

	return normalize(direction);
}

//Returns face index and UV coordinates
float lookupCubemapFrom_Direction(in float3 direction, out float2 UV)
{
	//Biggest component will determine cubemap face. Remaining components will determine UV coords

	float3 absDirection = abs(direction);

	float face;
	float2 _uv;
	//X set
	if (absDirection.x >= absDirection.y && absDirection.x >= absDirection.z)
	{
		bool isXpos = direction.x > 0.0f;
		face = isXpos  ?  0.0f  :  1.0f;

		_uv = float2(isXpos ? -direction.z : direction.z, -direction.y);
	}
	else if (absDirection.y >= absDirection.z) //Y set. We already know that absDirection.x is not the greatest. Test Y and Z
	{
		bool isYpos = direction.y > 0.0f;
		face = isYpos  ?  2.0f  :  3.0f;

		_uv = float2(direction.x, isYpos ? direction.z : -direction.z);
	}
	else
	{
		bool isZpos = direction.z > 0.0f;
		face = isZpos ? 4.0f : 5.0f;

		_uv = float2( isZpos ? direction.x : -direction.x, -direction.y);
	}

	//uv remap
	UV = _uv * 0.5f + 0.5f;
	return face;
}
#endif //_COMMON_HLSL