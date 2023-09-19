#include "pch.h"

#pragma once
#include "include/Math/Maths.h"
#include "include/Math/MathUtils.h"

namespace fth
{
	namespace math
	{
		const Transform Transform::Initial = {math::Vector3::Zero, math::Vector3::One, math::Quaternion::Identity};


		std::ostream& operator<<(std::ostream& stream, const Vector3& vec3D)
		{
			stream << "X: " << vec3D.x << "\tY: " << vec3D.y << "\tZ: " << vec3D.z << std::endl;

			return stream;
		}
	}


}