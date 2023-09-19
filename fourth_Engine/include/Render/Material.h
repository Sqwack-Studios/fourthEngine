#pragma once

namespace fth
{
	

	struct Material
	{
		Material() = default;
		constexpr Material(const math::Color& albedo, bool emissive, float specularity, float roughness):
			albedo(albedo),
			emissive(emissive),
			specularity(specularity),
			roughness(roughness)
		{

		}

		math::Color albedo;
		bool  emissive;
		float specularity; //specular factor K_s
		float roughness;
	};
}
