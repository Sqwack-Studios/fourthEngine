#pragma once

namespace fth
{
    struct Ray;
    struct HitResult;


	struct Plane : public DirectX::SimpleMath::Plane
	{
        Plane() = default;

        constexpr Plane(float ix, float iy, float iz, float iw) noexcept : 
            DirectX::SimpleMath::Plane(ix, iy, iz, iw)
            {}

        Plane(const math::Vector3& normal, float d) noexcept : DirectX::SimpleMath::Plane(normal, d) {}
        Plane(const math::Vector3& point1, const math::Vector3& point2, const math::Vector3& point3) noexcept : DirectX::SimpleMath::Plane(point1, point2, point3) {};
        Plane(const math::Vector3& point, const math::Vector3& normal) noexcept;
        explicit Plane(const math::Vector4& v) noexcept : DirectX::SimpleMath::Plane(v) {}
        explicit Plane(_In_reads_(4) const float* pArray) noexcept : DirectX::SimpleMath::Plane(pArray) {}
        Plane(DirectX::FXMVECTOR V) noexcept: DirectX::SimpleMath::Plane(V) {}
        Plane(const XMFLOAT4& p) noexcept : DirectX::SimpleMath::Plane(p) {}
        explicit Plane(const DirectX::XMVECTORF32& F) noexcept : DirectX::SimpleMath::Plane(F) {}




        bool Intersects(const Ray& ray, math::HitResult& outHit/*, uint16_t& outMaterialIndex*/) const;

	};
}
