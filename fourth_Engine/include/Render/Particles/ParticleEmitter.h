#pragma once
#include "include/Utils/SolidVector.h"

namespace fth
{
	struct BaseID;
	using TransformID = BaseID;
}
namespace fth::particles
{

	struct Particle
	{
		math::Color     tint;
		math::Vector3   position;
		float           rotation; //radians!!
		math::Vector3   speed;
		float           timeAlive;
		math::Vector2   size;
	};

	struct ParticleEmitter
	{
	public:
		ParticleEmitter(const math::Vector3& pos, float spawnRate, const math::Color& tint, float spawnRadius, Handle<math::Matrix> parent);


		math::Vector3          m_emitterPos;
		float                  m_spawnRate;//Particles/second
		math::Color            m_spawnTint;
		float                  m_spawnRadius;
		Handle<math::Matrix>            m_parentID;

		std::vector<Particle>  m_particles;
	};
}