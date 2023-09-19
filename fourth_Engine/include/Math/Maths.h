
#pragma once
#include "vendor/SimpleMath/SimpleMath.h"
#include "vendor/SimpleMath/DirectXCollision.h"
namespace fth
{
	struct IDragger;

	namespace math
	{
		using Vector2 = DirectX::SimpleMath::Vector2;
		using Vector3 = DirectX::SimpleMath::Vector3;
		using Vector4 = DirectX::SimpleMath::Vector4;
		using Matrix = DirectX::SimpleMath::Matrix;
		using Quaternion = DirectX::SimpleMath::Quaternion;
		using Color = DirectX::SimpleMath::Color;
		using AABB = DirectX::BoundingBox;


		struct Transform
		{
			static const Transform Initial;
			Vector3       position;
			Vector3       scale;
			Quaternion    rotation;

		};

		struct HitResult
		{
			math::Vector3              hitPoint;
			math::Vector3              hitNormal;
			float                      tDistance = std::numeric_limits<float>::infinity();
			void* hitObject;

			bool IsValid() { return std::isfinite(tDistance); }
			void ResetDistance() { tDistance = std::numeric_limits<float>::infinity(); }

		};


	}

	

}




























