#ifndef _GPU_PARTICLES_HLSLI
#define _GPU_PARTICLES_HLSLI

static const uint MAX_GPU_PARTICLES = 1024;


struct GPUParticle
{
	float4  emission;
	//
	float3  position;
	float   rotation;
	//
	float3  speed;
	float   timeLeft;
	//
	float2  size;
	float2  pad; //allign to 16bytes
};//developer.nvidia.com/content/understanding-structured-buffer-performance

//This will compute spawn index based on the current amount of particles, the offset and maximum amount of particles
//This function is useful if you want to keep spawning particles when budget is full, overwriting older particles.
uint computeSpawnIndex(in uint number, in uint offset, in uint MAX_PARTICLES)
{
	float MAX = max(MAX_PARTICLES, 1.0f);//just to get rid of X4008 division by zero ??
	return fmod(offset + number, MAX);
}
#endif //_GPU_PARTICLES_HLSLI