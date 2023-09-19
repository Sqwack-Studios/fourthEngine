#include "pch.h"

#pragma once
#include "include/Render/Primitive Shapes/Sphere.h"
#include "include/Math/Ray.h"


namespace fth
{

	Sphere::Sphere(const math::Vector3& pos, float radius) :
		DirectX::BoundingSphere(pos, radius)
	{

	}

	bool Sphere::Intersects(const Ray& ray, math::HitResult& outHit, uint16_t& outMaterialIndex) const
	{
		float t_found{};
		bool foundIntersection = DirectX::BoundingSphere::Intersects(ray.position, ray.direction, t_found);

		bool updateIntersection{ foundIntersection && t_found < outHit.tDistance };

		if (updateIntersection)
		{
			math::Vector3 hitPoint{ ray.at(t_found) };
			math::Vector3 hitNormal{ hitPoint - Center };
			hitNormal.Normalize();

			if ((ray.position - Center).Length() < Radius)
				hitNormal = -hitNormal;
			

			outHit.tDistance = t_found;

			outMaterialIndex = materialIndex;
			outHit.hitNormal = hitNormal;
			outHit.hitPoint = hitPoint;
			outHit.hitObject = const_cast<Sphere*>(this);
		}

		return foundIntersection;
	}




	

}
