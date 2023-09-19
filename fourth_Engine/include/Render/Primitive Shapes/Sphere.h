#pragma once

namespace fth
{
	struct HitResult;
	struct Ray;

	struct Sphere : public DirectX::BoundingSphere
	{
	public:
		Sphere() = default;
		Sphere(const math::Vector3& pos, float radius);


		bool Intersects(const Ray& ray, math::HitResult& outHit, uint16_t& outMaterialIndex) const;


		uint16_t       materialIndex;


		

	};
}