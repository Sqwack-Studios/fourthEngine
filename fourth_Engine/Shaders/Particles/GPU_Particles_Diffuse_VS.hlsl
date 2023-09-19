#include "gpuParticles.hlsli"
#include "../cbPerView.hlsli"

#ifdef PARTICLES_LIGHTNING_SPHERES
struct VSIn
{
	float3 lPos     : LPOSITION;
	uint instanceID : SV_InstanceID;
};

#elif PARTICLES_LIGHTNING_BILLBOARDS
#include "../Fullscreen_Pass.hlsli"
struct VSIn
{
	uint vertexID : SV_VertexID;
	uint instanceID : SV_InstanceID;
};
#endif


struct VSOut
{
	float4 clip : SV_POSITION;
	nointerpolation float  radius : RADIUS;
	nointerpolation float  size : SIZE;
	nointerpolation float3 punctualPos : PPOSITION;
	nointerpolation float4 emission : EMISSION;
};

ByteAddressBuffer g_rangeBuffer : register(srv_RANGE_BUFFER);
StructuredBuffer<GPUParticle> g_particles : register(srv_PARTICLE_BUFFER);


VSOut main(VSIn vin) 
{
	uint offset = g_rangeBuffer.Load(4);
	uint index = fmod(offset + vin.instanceID, MAX_GPU_PARTICLES);

	GPUParticle particle = g_particles.Load(index);

	float size = max(particle.size.x, particle.size.y);
	float scale = 10.0f * size;

	float3 punctualPos, pos;
	//punctualPos is the position of the particle
	//pos is the position of the vertex being sent to rasterize
	float4 clip;

#ifdef CAMERA_CENTER_WS
	punctualPos = particle.position - g_invV[3].xyz;
	row_major float3x3 cameraRotation = float3x3(g_V[0].xyz, g_V[1].xyz, g_V[2].xyz);

#endif

#ifdef PARTICLES_LIGHTNING_SPHERES

	pos = vin.lPos * scale + punctualPos;

#elif PARTICLES_LIGHTNING_BILLBOARDS
//for billboarding, we have to offset billboard position by the radius of the light towards the camera to prevent billboard clipping artifacts
//then, once we have computed view space, we expand quad vertex by the radius after they are rotated
	pos = punctualPos - normalize(punctualPos) * scale * 1.5f;

	float sin, cos;
	sincos(particle.rotation, sin, cos);
	row_major float2x2 rotateQuad = { float2(cos, -sin), float2(sin, cos) };
	float2 qVertex = BILLBOARD[vin.vertexID].position.xy;

	float2 offsetQuad = (scale * 1.5f) * mul(qVertex, rotateQuad);

#endif//PARTICLE_LIGHTNING



#ifdef CAMERA_CENTER_WS

	clip.xyz = mul(pos, cameraRotation);

#ifdef PARTICLES_LIGHTNING_BILLBOARDS
	clip.xy += offsetQuad;
#endif

	clip = mul(float4(clip.xyz, 1.0f), g_P);

#else

#endif//CAMERA_CENTER_WS

	VSOut vout;
	vout.clip = clip;
	vout.radius = scale;
	vout.size = size;
	vout.punctualPos = punctualPos;
	vout.emission = particle.emission;

	return vout;
}