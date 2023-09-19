#include "../Common.hlsli"
#include "../cbPerView.hlsli"
#include "../cbPerFrame.hlsli"

struct VSOut
{
	float4 clip                                   : SV_POSITION;
	nointerpolation row_major float4x4 worldDecal : WORLD_DECAL;
	nointerpolation float4 color                  : COLOR;
	nointerpolation uint   parentObjID            : PID;
};

Texture2D<float4> g_normalAlphaMap : register(srv_DECAL_NORMAL_ALPHA);
Texture2D<float4> g_normalGBuffer : register(srv_DECAL_NORMAL_GB);
Texture2D<float1> g_depthMap  : register(srv_DECAL_DEPTH);
Texture2D<uint1>  g_objectIDGBuffer : register(srv_DECAL_OBJECTID_GB);

cbuffer PerMaterial : register(cb_MATERIAL)
{
	//Bools are 4 bytes in hlsl. Considering that small buffers might be performance critical, use a single int to store booleans and perform bitwise operations to extract data?
	uint  g_flags; 
	//bit0 : invertNormal
	//bit1 : buildBlue
	
};

struct PS_OUT
{
	float4 albedo      : TARGET_ALBEDO;
	float4 rough_metal : TARGET_ROUGH_METAL;
	float4 normals     : TARGET_TX_GM_NORMAL;
	float4 emission    : TARGET_EMISSION;
};

PS_OUT main(VSOut vout) 
{
	{
		uint1 parentID = g_objectIDGBuffer.Load(uint3(vout.clip.xy, 0));
		if (parentID != vout.parentObjID)
		{
			discard;
		}
	}

	//Transform to DecalSpace and discard if necessary

	row_major float4x4 worldDecal = vout.worldDecal;
	float depth = g_depthMap.Load(uint3(vout.clip.xy, 0));
	float4 clip = float4(vout.clip.xy * g_resolution.zw, depth, 1.0f);
	//clip XY goes from [0, 1] right now. Expand to [-1, 1]
	clip.x =  clip.x * 2.0f - 1.0f;
	clip.y = -clip.y * 2.0f + 1.0f;

	float4 worldPos;
#ifdef CAMERA_CENTER_WS

	worldPos = mul(clip, g_invP);

	row_major float3x3 invRotationView = float3x3(float3(g_invV[0].xyz), float3(g_invV[1].xyz), float3(g_invV[2].xyz));
	worldPos.xyz = mul(worldPos.xyz, invRotationView);

#else

	mul(clip, g_invVP);

#endif
	worldPos /= worldPos.w;

	float3 decalSpace = mul(worldPos, worldDecal).xyz;

	//unit cube is 0.5 unis for each axis 
	bool passX = decalSpace.x < 0.5f && decalSpace.x > -0.5f;
	bool passY = decalSpace.y < 0.5f && decalSpace.y > -0.5f;
	bool passZ = decalSpace.z < 0.5f && decalSpace.z > -0.5f;

	bool isClipped = !(passX && passY && passZ);

	if (isClipped)
	{
		discard;
	}

	float2 texCoords;
	texCoords.x =  decalSpace.x + 0.5f;
	texCoords.y = -decalSpace.y + 0.5f;

	//decal might be scaled

	float4 packedNormals = g_normalGBuffer.Load(uint3(vout.clip.xy, 0));

	float3 microNormal = unpackOctahedron(packedNormals.xy);
	float3 T = normalize(float3(worldDecal[0].x, worldDecal[1].x, worldDecal[2].x));
	float3 B = normalize(cross(microNormal, T));

	row_major float3x3 TBN = { T, B, microNormal};

	bool invertNormal = (g_flags >> 0) & 1;
	bool buildBlue    = (g_flags >> 1) & 1;

	float4 normal_Alpha = g_normalAlphaMap.Sample(g_anisotropicWrap, texCoords);

	float3 decalNormalMap = unpackNormalMap(normal_Alpha.xyz, invertNormal, buildBlue);
	decalNormalMap = mul(decalNormalMap, TBN);

	float3 decalForward = float3(worldDecal[0].z, worldDecal[1].z, worldDecal[2].z);

	{
		float dNoN = dot(microNormal, decalForward);

		if (dNoN > 0.0f)
		{
			discard;
		}
	}

	decalNormalMap = normalize(lerp(microNormal, decalNormalMap, normal_Alpha.a));


	float4 outNormals = float4(packOctahedron(decalNormalMap), packedNormals.zw);

	PS_OUT psout = { float4(vout.color.rgb, normal_Alpha.a), float4(1.0f, 0.0f, 0.0f, normal_Alpha.a), outNormals, float4(0.0f, 0.0f, 0.0f, normal_Alpha.a) };

	return psout;
}