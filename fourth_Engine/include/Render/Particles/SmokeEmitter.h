#pragma once
#include "include/Render/Particles/ParticleEmitter.h"

namespace fth::particles
{
	struct SmokeEmitter : public ParticleEmitter
	{
	public:
		struct ParticleInstanceGPU
		{
			math::Vector4       tint;
			math::Vector3       pos;
			float               rotation;
			math::Vector3       speed;
			float               timeFraction;
			math::Vector2       size;
			uint32_t            tileIdx;
		};

		SmokeEmitter(const math::Vector3& pos,
			float spawnRate,
			const math::Color& tint,
			float spawnRadius,
			Handle<math::Matrix> parentID,
			math::Vector2 xSpeedRange, 
			math::Vector2 zSpeedRange,  
			math::Vector2 initialSize,
			math::Vector2 rotationRange, 
			float sizeIncreaseRate,
			float ySpeed, 
			float particleLifetime,
			float maxParticles);

		void Update(float deltaTime);
		uint32_t NumParticlesSpawned() const { return static_cast<uint32_t>(m_particles.size()); }
	private:
		Particle GenerateParticle();
	public:
		math::Vector2   m_XspeedRange;//Horizontal Speeds range
		math::Vector2   m_ZspeedRange;
		math::Vector2   m_initialSize;
		math::Vector2   m_rotationRange;
		float           m_sizeIncreaseRate;
		float           m_Yspeed;
		float           m_particleLifetime;
		float           m_particleFrameTime = 0.0f;
		float           m_deltaTimeAccumulator = 0.0f;
		uint32_t        m_maxParticles;
		uint32_t        m_currentAmountParticles;
	};
}