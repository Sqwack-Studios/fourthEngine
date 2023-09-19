#include "pch.h"
#pragma once
#include "include/Render/Particles/ParticleEmitter.h"

namespace fth::particles
{
	ParticleEmitter::ParticleEmitter(const math::Vector3& pos, float spawnRate, const math::Color& tint, float spawnRadius, Handle<math::Matrix> parent):
		m_emitterPos(pos),
		m_spawnRate(spawnRate),
		m_spawnTint(tint),
		m_spawnRadius(spawnRadius),
		m_parentID(parent)
	{
	}
}