#include "../FullScreen_Pass.hlsli"
#include "gpuParticles.hlsli"
#include "../cbPerView.hlsli"

ByteAddressBuffer g_rangeBuffer : register(srv_RANGE_BUFFER);
StructuredBuffer<GPUParticle> g_particles : register(srv_PARTICLE_BUFFER);


struct VSOut
{
	float4 clip : SV_POSITION;
	float2 uv : TEXCOORDS;
	nointerpolation float4 emission : EMISSION;
};

VSOut main(uint vtxID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	uint offset = g_rangeBuffer.Load(4);
	uint index = fmod(offset + instanceID, MAX_GPU_PARTICLES);

	GPUParticle particle = g_particles.Load(index);

	float sin, cos;
	sincos(particle.rotation, sin, cos);
	row_major float2x2 rotateOffsets = { float2(cos, -sin), float2(sin, cos) };

	Vertex billboardVertex = BILLBOARD[vtxID];

	float4 clip;
	float3 pos;
#if CAMERA_CENTER_WS
	row_major float3x3 cameraRotation = float3x3(g_V[0].xyz, g_V[1].xyz, g_V[2].xyz);
	pos = particle.position - g_invV[3].xyz;

	clip.xyz = mul(pos, cameraRotation);
	clip.xy += particle.size * mul(billboardVertex.position.xy, rotateOffsets);
	clip = mul(float4(clip.xyz, 1.0f), g_P);

#else


#endif

	VSOut vout;
	vout.clip = clip;
	vout.uv = billboardVertex.uv;
	vout.emission = particle.emission;
	return vout;
}