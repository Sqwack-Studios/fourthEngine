#include "pch.h"

#pragma once
#include "include/Render/Primitive Shapes/Plane.h"
#include "include/Math/Ray.h"
#include "include/Math/MathUtils.h"

namespace fth
{
	bool Plane::Intersects(const Ray& ray, math::HitResult& outHit/*, uint16_t& outMaterialIndex*/) const
	{
		float t_found{};
		bool foundIntersection = DirectX::SimpleMath::Plane::Intersects(ray.position, ray.direction, t_found);

		bool updatedIntersection{ foundIntersection && t_found < outHit.tDistance };

		if (updatedIntersection)
		{
			outHit.tDistance = t_found;
			outHit.hitObject = const_cast<Plane*>(this);
			outHit.hitPoint = ray.at(t_found);
			math::Vector3 planeNormal{ Normal() };

			if (DotNormal(ray.direction) > 0.0f)
				outHit.hitNormal = -planeNormal;
			else
				outHit.hitNormal = planeNormal;

			/*outMaterialIndex = materialIndex;*/
		}

		return foundIntersection;
	}


	
}