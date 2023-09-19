#include "pch.h"
#pragma once
#include "include/Render/Particles/SmokeEmitter.h"
#include "include/Utils/Random.h"
#include "include/Systems/TransformSystem.h"
#include "include/Managers/CameraManager.h"
namespace fth::particles
{

	SmokeEmitter::SmokeEmitter(
		const math::Vector3& pos,
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
		float maxParticles) :

		ParticleEmitter(pos, spawnRate, tint, spawnRadius, parentID),
		m_XspeedRange(xSpeedRange),
		m_ZspeedRange(zSpeedRange),
		m_initialSize(initialSize),
		m_rotationRange(rotationRange),
		m_sizeIncreaseRate(sizeIncreaseRate),
		m_Yspeed(ySpeed),
		m_particleLifetime(particleLifetime),
		m_deltaTimeAccumulator(0.0f),
		m_maxParticles(maxParticles),
		m_currentAmountParticles(0)
	{
		m_particles.reserve(maxParticles);
	}

	void SmokeEmitter::Update(float deltaTime)
	{
		uint32_t particlesInMemory{ static_cast<uint32_t>(m_particles.size()) };
		bool budgetIsFull{ particlesInMemory >= m_maxParticles };
//Let's say for now that for 1/10 of the lifetime, alpha increases linearly.
//From 1/10 to 6/10 stays constant and from 6/10 to end of lifetime decreases linearly

		constexpr float alphaIncreaseInterval = 0.1f;
		constexpr float alphaconstantInterval = 0.6f;

		const float alphaIncreaseTime = m_particleLifetime * alphaIncreaseInterval;
		const float invAlphaIncreaseTime = 1.0f / alphaIncreaseTime;
		const float alphaConstantTime = m_particleLifetime * alphaconstantInterval;
		const float invAlphaConstantMinusIncreaseTime = 1.0f / (alphaConstantTime - alphaIncreaseTime);

//Particles that should spawn in this deltaTime. 

		m_deltaTimeAccumulator += deltaTime;
		float spawnInterval{ 1.0f / m_spawnRate };

		const uint32_t particlesByDelta{ static_cast<uint32_t>(m_deltaTimeAccumulator / spawnInterval) };
		if (particlesByDelta)
		{
			m_deltaTimeAccumulator = m_deltaTimeAccumulator - spawnInterval * particlesByDelta;
		}
		uint32_t particlesToSpawn{ particlesByDelta };


		//first update current particles
		for (uint32_t i = 0; i < particlesInMemory; ++i)
		{
			Particle& particle = m_particles[i];
			particle.timeAlive += deltaTime;
			if (particle.timeAlive > m_particleLifetime)
			{
				if (particlesToSpawn)
				{
					m_particles[i] = GenerateParticle();
				}
				else
				{
					--m_currentAmountParticles;
				}
				continue;
			}

			particle.position += particle.speed * deltaTime;
			particle.size += math::Vector2::One * m_sizeIncreaseRate * deltaTime;

			//Update particle alpha based on time alive using precomputed factors

			if (particle.timeAlive < alphaIncreaseTime)
			{
				particle.tint.A() = particle.timeAlive * invAlphaIncreaseTime;
			}
			else if(particle.timeAlive > alphaConstantTime)
			{
				particle.tint.A() = (m_particleLifetime - particle.timeAlive ) * invAlphaConstantMinusIncreaseTime;
			}

		}

		if (particlesToSpawn && !budgetIsFull)
		{
			for (uint32_t i = 0; i < particlesToSpawn; ++i)
			{
				m_particles.push_back(GenerateParticle());
				++m_currentAmountParticles;
			}
		}
	}

	Particle SmokeEmitter::GenerateParticle()
	{
		//To generate a particle, we need random position inside a circle, a random speed and a random rotation
		math::Vector2 randomHorizontal = Random::randomVector2(m_XspeedRange, m_ZspeedRange);

		math::Vector3 speed(randomHorizontal.x, m_Yspeed, randomHorizontal.y);
		float angle{ Random::randomFloatUniformDistribution(m_rotationRange.x, m_rotationRange.y) };

		//Generate a random X point that inside a circle centered around (0, 0)
		//Random Z point will be limited by X
		float xPos{ Random::randomFloatUniformDistribution(-m_spawnRadius, m_spawnRadius) };

		float zRange{ std::sqrtf(m_spawnRadius * m_spawnRadius - xPos * xPos) };
		float zPos{ Random::randomFloatUniformDistribution(-zRange, zRange) };

		math::Matrix parentTransform = m_parentID.isValid() ? TransformSystem::Get().QueryTransformMatrix(m_parentID) : math::Matrix::Identity;
		math::Vector3 pos{m_emitterPos};
		pos.x += xPos;
		pos.z += zPos;
		//ignore parent scaling
		{
			math::Vector3 parentRight = parentTransform.Right();
			parentRight.Normalize();
			parentTransform.Right(parentRight);
			math::Vector3 parentUp = parentTransform.Up();
			parentUp.Normalize();
			parentTransform.Up(parentUp);
			math::Vector3 parentForward = parentTransform.Forward();
			parentForward.Normalize();
			parentTransform.Forward(parentForward);

			pos = math::Vector3::Transform(pos, parentTransform);
		}

#ifdef CAMERA_CENTER_WS
		pos - CameraManager::GetActiveCamera().GetPosition();
#endif
		
		math::Vector2 initialSize{ m_initialSize };

		math::Color tint{ m_spawnTint };
		tint.A() = 0.0f;
		++m_currentAmountParticles;
		return { tint, pos, angle, speed, 0.0f, initialSize};
	}


}