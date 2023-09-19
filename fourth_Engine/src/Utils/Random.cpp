#include "pch.h"
#pragma once
#include "include/Utils/Random.h"
#include <random>

namespace fth
{
	//not thread safe, use thread_local for to make per-thread instance
	std::mt19937       rng;


	void Random::Init()
	{
		rng.seed(std::random_device()());
	}

	float Random::randomFloatUniformDistribution(float min, float max)
	{
		std::uniform_real_distribution<float> distribution(min, max);
		return  distribution(rng);
	}
	double Random::randomDoubleUniformDistribution(double min, double max)
	{
		std::uniform_real_distribution<double> distribution(min, max);
		return  distribution(rng);
	}
	uint16_t Random::randomU16UniformDistribution(uint16_t min, uint16_t max)
	{
		std::uniform_int_distribution<uint16_t> distribution(min, max);
		return  distribution(rng);
	}
	uint32_t Random::randomU32UniformDistribution(uint32_t min, uint32_t max)
	{
		std::uniform_int_distribution<uint32_t> distribution(min, max);
		return  distribution(rng);
	}
	uint64_t Random::randomU64UniformDistribution(uint64_t min, uint64_t max)
	{
		std::uniform_int_distribution<uint64_t> distribution(min, max);
		return  distribution(rng);
	}
	math::Vector2 Random::randomVector2(math::Vector2 xRange, math::Vector2 yRange)
	{
		return math::Vector2(randomFloatUniformDistribution(xRange.x, xRange.y), randomFloatUniformDistribution(yRange.x, yRange.y));
	}
	math::Vector3 Random::randomVector3(math::Vector2 xRange, math::Vector2 yRange, math::Vector2 zRange)
	{
		return math::Vector3(randomFloatUniformDistribution(xRange.x, xRange.y), randomFloatUniformDistribution(yRange.x, yRange.y), randomFloatUniformDistribution(zRange.x, zRange.y));
	}
	math::Vector4 Random::randomVector4(math::Vector2 xRange, math::Vector2 yRange, math::Vector2 zRange, math::Vector2 wRange)
	{
		return math::Vector4(randomFloatUniformDistribution(xRange.x, xRange.y), randomFloatUniformDistribution(yRange.x, yRange.y), 
			randomFloatUniformDistribution(zRange.x, zRange.y), randomFloatUniformDistribution(wRange.x, wRange.y));
	}
}