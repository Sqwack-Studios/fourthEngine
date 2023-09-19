#pragma once


namespace fth
{
	//This will need to be improved using a single uint generator from [-MAX, MAX] and scaling results accordingly
	struct Random
	{
		static void Init();
		static float randomFloatUniformDistribution(float min, float max);
		static double randomDoubleUniformDistribution(double min, double max);
		static uint16_t randomU16UniformDistribution(uint16_t min, uint16_t max);
		static uint32_t randomU32UniformDistribution(uint32_t min, uint32_t max);
		static uint64_t randomU64UniformDistribution(uint64_t min, uint64_t max);
		static math::Vector2 randomVector2(math::Vector2 xRange, math::Vector2 yRange);
		static math::Vector3 randomVector3(math::Vector2 xRange, math::Vector2 yRange, math::Vector2 zRange);
		static math::Vector4 randomVector4(math::Vector2 xRange, math::Vector2 yRange, math::Vector2 zRange, math::Vector2 wRange);

	};
}