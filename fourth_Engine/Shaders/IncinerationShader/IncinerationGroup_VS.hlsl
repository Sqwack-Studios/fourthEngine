#include "IncinerationEffect.hlsli"
#include "../cbPerView.hlsli"
#include "../cbPerFrame.hlsli"

#include "../IBL/HemisphereSampling.hlsli"//to get Van der Corput sequence
struct VSIn
{
	//Model data
	float3 lPos       : POSITION;
	float3 lNormal    : NORMAL;
	float3 lTangent   : TANGENT;
	float3 lBitangent : BITANGENT;
	float2 uv         : UV;
	//Instance Data
	float4 MWRight    : MW0;
	float4 MWUp       : MW1;
	float4 MWFwd      : MW2;
	float4 MWPos      : MW3;

	float3 emission   : EMISSION;
	float3 spherePos  : SPHEREPOS;//sphere position in model space
	uint   objectID   : OBJECTID;
	float2 sphereRadius : SPHRADIUS; //x: radius, y: prevRadius
};

//struct VSOutGeometry
//{
//	float4 clip      : SV_POSITION;
//	float3 pos       : POSITION;
//	float3 normal    : NORMAL;
//	float3 tangent   : TANGENT;
//	float3 bitangent : BITANGENT;
//};
//
//struct VSOutIncineration
//{
//	float distanceSq : DISTANCESQ;
//	nointerpolation float currentRadiusSq : CRADIUSQ;
//	nointerpolation float prevRadiusSq : PRADIUSQ;
//	nointerpolation float4 color : COLOR;
//};
//
//struct VSOut
//{
//	VSOutGeometry geo;
//	VSOutIncineration inc;
//	nointerpolation uint objectID : OBJECTID;
//};


struct VSOut
{
	float4 clip                           : SV_POSITION;
	float3 pos                            : POSITION;
	float3 normal                         : NORMAL;
	float2 uv                             : UV;
	nointerpolation float3  spherePos     : SPHEREPOS;
	nointerpolation uint objectID         : OBJECTID;
	float3 tangent                        : TANGENT;
	nointerpolation float currentRadius   : CRADIUS;
	float3 bitangent                      : BITANGENT;
	nointerpolation float prevRadius      : PRADIUS;
	nointerpolation float3 emission       : EMISSION;
};


cbuffer PerDraw : register(cb_PER_DRAW)
{
	row_major float4x4 g_meshToModel;
	float              g_maxRadius;
	float              padding[3];
};


RWByteAddressBuffer              g_rangeBuffer  : register(uav_RANGE_BUFFER);
RWStructuredBuffer<GPUParticle>  g_particlesUAV : register(uav_PARTICLES_BUFFER);

VSOut main( VSIn vin, uint vtxID : SV_VERTEXID ) 
{
	
//1-Transform sphere pos to world space. Vertex comes in modelSpace. We can avoid an operation by sending vertex in modelSpace
//2-Check for particle spawn and spawn
//3-Transform vertex to world space
//4-If spawn, load offset and number, increase number atomically and compute spawnIndex
//5-Spawn
//6-Send to rasterize
	row_major float4x4 modelToWorld = float4x4(vin.MWRight, vin.MWUp, vin.MWFwd, vin.MWPos);
	float4x4 meshToModel = float4x4(g_meshToModel);
	float4x4 worldToClip = float4x4(g_VP);

	
	float3 modelPos = mul(float4(vin.lPos, 1.0f), meshToModel).xyz;

	float3 worldPos = mul(float4(modelPos, 1.0f), modelToWorld).xyz;

	float3 normalModel = mul(float4(vin.lNormal.xyz, 0.0f), meshToModel).xyz;
	float3 tangentModel = mul(float4(vin.lTangent.xyz, 0.0f), meshToModel).xyz;
	float3 bitangentModel = mul(float4(vin.lBitangent.xyz, 0.0f), meshToModel).xyz;

	float3 normal, tangent, bitangent;

	normal = transformVector(normalModel, modelToWorld);
	tangent = transformVector(tangentModel, modelToWorld);
	bitangent = transformVector(bitangentModel, modelToWorld);

	float scale = max(length(modelToWorld[0].xyz), max(length(modelToWorld[1].xyz), length(modelToWorld[2].xyz)));

	float3 radiuses =  { vin.sphereRadius.xy, g_maxRadius};
	radiuses *= scale;
	float3 spherePos = mul(float4(vin.spherePos, 1.0f), modelToWorld).xyz;


	float vertexRadius = length(spherePos - worldPos);
	{
		IESphereParams sphereParams;
		sphereParams.vertexRadius = vertexRadius;
		sphereParams.currentRadius = radiuses.x;
		sphereParams.prevRadius = radiuses.y;
		sphereParams.maxRadius = radiuses.z;

		static const float SPAWN_VTX_MULTIPLEOF = 128.0f;
		static const float initialSpeed = 2.0f; //3m/s
		static const float initialLifetime = 10.0f; // 1s
		static const float2 initialSize = float2( 0.050f, 0.050f);

		bool shouldSpawnParticle = IEcheckSpawn(sphereParams, vtxID * g_time, SPAWN_VTX_MULTIPLEOF);

		if (shouldSpawnParticle)
		{
			uint number;
			uint offset = g_rangeBuffer.Load(4);//1st element is 4bytes size
			g_rangeBuffer.InterlockedAdd(0, 1, number);

			uint spawnIdx = computeSpawnIndex(number, offset, MAX_GPU_PARTICLES);

			GPUParticle particle;
			particle.emission = float4(vin.emission, 1.0f);
#ifdef CAMERA_CENTER_WS
			particle.position = worldPos + g_invV[3].xyz;
#else
			particle.position = worldPos;
#endif
			particle.rotation = _2PI * randomVanDeCorput(vtxID);
			particle.speed = initialSpeed * normal;
			particle.timeLeft = initialLifetime;
			particle.size = initialSize;
			particle.pad = 0.0f;
			g_particlesUAV[spawnIdx] = particle;
		}
	}

	

	float4 clip;
#ifdef CAMERA_CENTER_WS
	row_major float3x3 cameraRotation = float3x3(g_V[0].xyz, g_V[1].xyz, g_V[2].xyz);
	clip = mul(float4(mul(worldPos, cameraRotation), 1.0f), g_P);
#else
	clip = mul(float4(worldPos, 1.0f), g_VP);
#endif



	VSOut vout;

	vout.clip = clip;
	vout.pos = worldPos;
	vout.spherePos = spherePos;
	vout.normal = normal;
	vout.objectID = vin.objectID;
	vout.tangent = tangent;
	vout.currentRadius = radiuses.x;
	vout.bitangent = bitangent;
	vout.prevRadius = radiuses.y;
	vout.emission = vin.emission;
	vout.uv = vin.uv;

	return vout;
}
