#pragma once

namespace fth
{

	struct Ray
	{
		//A ray contains a origin (point), a direction (unit vector) and travel distance (magnitude)
		//We could set a max travel distance, or compute for any distance

		// Ray = origin + t * dir
		Ray() = default;

		Ray(const math::Vector3& origin, const math::Vector3& direction):
			position(origin),
			direction(direction)
		{

		}

		math::Vector3 at(float t) const
		{
			return position + t * direction;
		}

		math::Vector3 position;
		math::Vector3 direction;

	

	};

}

