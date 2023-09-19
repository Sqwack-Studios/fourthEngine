#include "../Common.hlsli"
#include "gpuParticles.hlsli"
#include "../cbPerView.hlsli"
#include "../cbPerFrame.hlsli"

#ifndef _UPDATE_PSIM_RINGBUFFER_CS_HLSLI
#define _UPDATE_PSIM_RINGBUFFER_CS_HLSLI

RWStructuredBuffer<GPUParticle>  g_ringBuffer : register(uav_PSIM_RING_BUFFER);
RWByteAddressBuffer              g_rangeBuffer: register(uav_PSIM_RANGE_BUFFER);

Texture2D<float1>                g_depthBuffer : register(srv_PSIM_DEPTH);
Texture2D<float4>                g_normalsBuffer : register(srv_PSIM_NORMALS_GB);

//Simulates 64 particles per thread group, thread per particle
//Each thread group has a global index that we can use to check if we actually dont
//need to run this shader because previous shaders made the work


[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint2 numOffset = g_rangeBuffer.Load2(0);
	if (DTid.x >= numOffset.x || DTid.x >= MAX_GPU_PARTICLES)
		return;


	//uint index = numOffset.y + fmod(numOffset.x, max(1.0f, MAX_GPU_PARTICLES));
	uint index = fmod(numOffset.y + DTid.x, max(1.0f, MAX_GPU_PARTICLES));

	//uint index = DTid.x;

	static const float3 GRAVITY = float3(0.0f, -9.81f, 0.0f);
	static const float  RESTITUTION_COEFICIENT = 0.5f; //speed(energy) lost each bounce
	static const float  SURFACE_THICKNESS = 0.1f;

	//Read particle
	GPUParticle particle = g_ringBuffer[index];

	particle.timeLeft -= g_deltaTime;

	if (particle.timeLeft > 0.0f)//update the particle
	{
		static const float rotationSpeed = 0.1f; //0.1 rad/s

		particle.rotation += rotationSpeed * g_deltaTime;

		float3 updatedPos, updatedSpeed;
		updatedPos = particle.position;
		updatedSpeed = particle.speed;


		updatedSpeed += GRAVITY * g_deltaTime;
		updatedPos += particle.speed * g_deltaTime;


		float speedMag = length(updatedSpeed);
		float3 normalizedSpeed = updatedSpeed / speedMag;
//OLD METHOD
		//float4 clipSpace;

		//clipSpace = mul(float4(updatedPos, 1.0f), g_VP);
		//clipSpace /= clipSpace.w;


		////Convert XY [-1, 1] into [0, PIXELS -1]
		//float2 texCoords;
		//texCoords.x = (clipSpace.x + 1.0f) * (g_resolution.x - 1) / 2;
		//texCoords.y = (1.0f - clipSpace.y) * (g_resolution.y - 1) / 2;

		//float sceneLinearDepth = linearizeDepth(g_depthBuffer.Load(uint3(texCoords, 0)), g_nearPlane, g_farPlane);
		//float particleLinearDepth = linearizeDepth(clipSpace.z, g_nearPlane, g_farPlane);
		//float3 normal = unpackOctahedron(g_normalsBuffer.Load(uint3(texCoords, 0)).xy);

		//float offset = sceneLinearDepth - particleLinearDepth;

		////try with adaptative thickness
		////compute which distance can travel projectile based on deltatime and speed
		////thats the maximum thickness of the surface, which then, would be weighted by projectile NoS

		////float thickness = max(speedMag * g_deltaTime * dot(normal, normalizedSpeed), SURFACE_THICKNESS);
		//float thickness = max(0.0f, SURFACE_THICKNESS);


		//if ( offset < thickness)
		//{
		//	//throw away previously updated position and speed
		//
		//	updatedSpeed = reflect(particle.speed, normal) * RESTITUTION_COEFICIENT;
		//	updatedPos = (particle.position) + particle.speed * g_deltaTime;
		//}

		//particle.position = updatedPos;
		//particle.speed = updatedSpeed;



//TODO: improve collision
//1st- we need to sample not in the same clip position as the particle, because the particle has certain speed.
//2nd- compute sampling position by offseting updated position by normalized speed and particle size
//3rd- compute sample world position
//4th- compute distanc between updated pos and scene pos, check if its greater than surface thickness
//5th- make bounce or whatever you want ?

		float size = max(particle.size.x, particle.size.y);
		float3 samplingPos = updatedPos + normalizedSpeed * size;

		float4 sclipSpace;

		sclipSpace = mul(float4(samplingPos, 1.0f), g_VP);
		sclipSpace /= sclipSpace.w;

		bool shouldCollide = -1.0f < sclipSpace.x && sclipSpace.x < 1.0f &&
			                 -1.0f < sclipSpace.y && sclipSpace.y < 1.0f;

		if (shouldCollide)
		{
			//Convert XY [-1, 1] into [0, PIXELS -1]
			float2 stexCoords;
			stexCoords.x = (sclipSpace.x + 1.0f) * (g_resolution.x - 1) / 2;
			stexCoords.y = (1.0f - sclipSpace.y) * (g_resolution.y - 1) / 2;

			float3 normal = unpackOctahedron(g_normalsBuffer.Load(uint3(stexCoords, 0)).xy);
			float4 scenePos = mul(float4(sclipSpace.xy, g_depthBuffer.Load(uint3(stexCoords, 0)), 1.0f), g_invVP);
			scenePos /= scenePos.w;

			float thickness = max(0.0f, SURFACE_THICKNESS);
			//float thickness = max(speedMag * g_deltaTime * dot(normal, normalizedSpeed), SURFACE_THICKNESS);
			float distance = length(updatedPos - scenePos.xyz);

			if (distance < thickness)
			{
				//throw away previously updated position and speed
				updatedSpeed = reflect(particle.speed, normal) * RESTITUTION_COEFICIENT;
				updatedPos = (particle.position) + particle.speed * g_deltaTime;
			}
		}


		particle.position = updatedPos;
		particle.speed = updatedSpeed;
	}
	else
	{
		uint prevExpired;
		g_rangeBuffer.InterlockedAdd(8, 1, prevExpired);
	}

	g_ringBuffer[index] = particle;
}



#endif //_UPDATE_PSIM_RINGBUFFER_CS_HLSLI