#pragma once
#include "include/Utils/NonCopyable.h"

namespace fth
{
	struct Ray;

	class Mesh;

	struct MeshIntersection
	{
		math::Vector3 pos;
		math::Vector3 normal;
		float tNear;
		float t;
		uint32_t triangle;

		constexpr void reset(float Near, float Far = std::numeric_limits<float>::infinity())
		{
			this->tNear = Near;
			t = Far;
		}
		bool valid() const { return std::isfinite(t); }
	};

	class TriangleOctree : public NonCopyable
	{
	public:
		const static int PREFFERED_TRIANGLE_COUNT;
		const static float MAX_STRETCHING_RATIO;

		TriangleOctree();
		TriangleOctree(TriangleOctree&& otherOctree);


		void clear() { m_mesh = nullptr; }
		bool inited() const { return m_mesh != nullptr; }

		void initialize(const Mesh& mesh);

		bool intersect(const Ray& ray, MeshIntersection& nearest) const;

	protected:
		const Mesh* m_mesh;
		std::vector<uint32_t> m_triangles;

		math::AABB m_box;
		math::AABB m_initialBox;

		std::unique_ptr<std::array<TriangleOctree, 8>> m_children;

		void initialize(const Mesh& mesh, const math::AABB& parentBox, const math::Vector3& parentCenter, int octetIndex);

		bool addTriangle(uint32_t triangleIndex, const math::Vector3& V1, const math::Vector3& V2, const math::Vector3& V3, const math::Vector3& center);

		bool intersectInternal(const Ray& ray, MeshIntersection& nearest) const;
	};

}
