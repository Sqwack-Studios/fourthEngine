#include "../Registers.hlsli"
#include "gpuParticles.hlsli"

RWByteAddressBuffer g_rangeBuffer        : register(uav_PSIM_RANGE_UPDATE);


//Byte Layout:
//Address is (ByteNum - 1) * 4
//1st byte: number
//2nd byte: offset
//3rd byte: expired
//4th byte: indexCountPerInstance
//-------------------
//5th byte: InstanceCount
//6th byte: StartIndexLocation
//7th byte: BaseVertexLocation
//8th byte: StartInstanceLocation
//ifdef PARTICLES_LIGHTNING_SPHERES
//9th  byte: indexCountPerSphereInstance
//10th byte: InstanceCount
//11th byte: StartIndexLocation
//12th byte: BaseVertexLocation
//13th byte: StartInstanceLocation

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint3 num_off_ex = g_rangeBuffer.Load3(0);

	uint number, offset, expired;
	number  = num_off_ex.x;
	offset  = num_off_ex.y;
	expired = num_off_ex.z;

	//I also take into account number because I dont stop spawning particles when budget is full. We overwrite old particles, so offset must account for extra particles.
	uint newOffset = offset;

	if (expired)
	{
		if (number > MAX_GPU_PARTICLES)
		{
			newOffset = fmod(offset + fmod(number, max(1.0f, MAX_GPU_PARTICLES)) + expired, max(1.0f, MAX_GPU_PARTICLES));
		}
		else
		{
			newOffset = fmod(offset + expired, max(1.0f, MAX_GPU_PARTICLES));
		}
	}
	else if(number > MAX_GPU_PARTICLES)
	{
		newOffset = fmod(offset + fmod(number, max(1.0f, MAX_GPU_PARTICLES)), max(1.0f, MAX_GPU_PARTICLES));
	}

	uint newNumber = min(number, MAX_GPU_PARTICLES) - expired;

	g_rangeBuffer.Store3(0, uint3(newNumber, newOffset, 0));
	g_rangeBuffer.Store(4 * 4, newNumber);//updates instanceCount

#ifdef PARTICLES_LIGHTNING_SPHERES
	g_rangeBuffer.Store(4 * 9, newNumber);
#endif
}