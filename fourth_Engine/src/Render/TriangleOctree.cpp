#include "pch.h"

#pragma once
#include "include/Render/TriangleOctree.h"
#include "include/Math/Ray.h"
#include "include/Render/Mesh.h"

namespace fth
{
	const int TriangleOctree::PREFFERED_TRIANGLE_COUNT = 32;
	const float TriangleOctree::MAX_STRETCHING_RATIO = 1.05f;

	inline const math::Vector3& getPos(const Mesh& mesh, uint32_t triangleIndex, uint32_t vertexIndex)
	{
		uint32_t index = mesh.m_meshRange.indexNum == 0 ? 
			triangleIndex * 3 + vertexIndex :
			mesh.GetTriangle(triangleIndex).indexes[vertexIndex];

		return mesh.GetVertex(index).position;
	}

	TriangleOctree::TriangleOctree(){}

	TriangleOctree::TriangleOctree(TriangleOctree&& otherOctree) :
		m_mesh(otherOctree.m_mesh),
		m_triangles(std::move(otherOctree.m_triangles)),
		m_box(otherOctree.m_box),
		m_initialBox(otherOctree.m_initialBox),
		m_children(std::move(otherOctree.m_children))

	{
		otherOctree.m_mesh = nullptr;
	}

	void TriangleOctree::initialize(const Mesh& mesh)
	{
		m_triangles.clear();
		m_triangles.shrink_to_fit();

		m_mesh = &mesh;
		m_children = nullptr;

		const math::Vector3 eps = { 1e-5f, 1e-5f, 1e-5f };
	
		m_box = m_initialBox = { mesh.GetAABB().Center, mesh.GetAABB().Extents + eps };
		/*m_box = m_initialBox = { mesh.box.min - eps, mesh.box.max + eps };*/
		uint32_t amountTriangles{ mesh.GetNumOfTriangles() };

		for (uint32_t i = 0; i < amountTriangles; ++i)
		{
			const math::Vector3& V1 = getPos(mesh, i, 0);
			const math::Vector3& V2 = getPos(mesh, i, 1);
			const math::Vector3& V3 = getPos(mesh, i, 2);

			math::Vector3 P = (V1 + V2 + V3) / 3.f;

			bool inserted = addTriangle(i, V1, V2, V3, P);
			FTH_ASSERT_ENGINE(inserted, "");
		}
	}

	void TriangleOctree::initialize(const Mesh& mesh, const math::AABB& parentBox, const math::Vector3& parentCenter, int octetIndex)
	{
		m_mesh = &mesh;
		m_children = nullptr;

		const float eps = 1e-5f;

		if (octetIndex % 2 == 0)
		{
			float halfParentExtentsX{ parentBox.Extents.x * 0.5f };
			m_initialBox.Center.x = parentBox.Center.x - halfParentExtentsX;
			m_initialBox.Extents.x = halfParentExtentsX;



			/*m_initialBox.min[0] = parentBox.min[0];
			m_initialBox.max[0] = parentCenter[0];*/
		}
		else
		{
			float halfParentExtentsX{ parentBox.Extents.x * 0.5f };
			m_initialBox.Center.x = parentBox.Center.x + halfParentExtentsX;
			m_initialBox.Extents.x = halfParentExtentsX;
			

			/*m_initialBox.min[0] = parentCenter[0];
			m_initialBox.max[0] = parentBox.max[0];*/
		}

		if (octetIndex % 4 < 2)
		{
			float halfParentExtentsY{ parentBox.Extents.y * 0.5f };
			m_initialBox.Center.y = parentBox.Center.y - halfParentExtentsY;
			m_initialBox.Extents.y = halfParentExtentsY;

			/*m_initialBox.min[1] = parentBox.min[1];
			m_initialBox.max[1] = parentCenter[1];*/
		}
		else
		{
			float halfParentExtentsY{ parentBox.Extents.y * 0.5f };
			m_initialBox.Center.y = parentBox.Center.y + halfParentExtentsY;
			m_initialBox.Extents.y = halfParentExtentsY;

			/*m_initialBox.min[1] = parentCenter[1];
			m_initialBox.max[1] = parentBox.max[1];*/
		}

		if (octetIndex < 4)
		{
			float halfParentExtentsZ{ parentBox.Extents.z * 0.5f };
			m_initialBox.Center.z = parentBox.Center.z - halfParentExtentsZ;
			m_initialBox.Extents.z = halfParentExtentsZ;
			/*m_initialBox.min[2] = parentBox.min[2];
			m_initialBox.max[2] = parentCenter[2];*/
		}
		else
		{
			float halfParentExtentsZ{ parentBox.Extents.z * 0.5f };
			m_initialBox.Center.z = parentBox.Center.z + halfParentExtentsZ;
			m_initialBox.Extents.z = halfParentExtentsZ;
			

			/*m_initialBox.min[2] = parentCenter[2];
			m_initialBox.max[2] = parentBox.max[2];*/
		}

		m_box = m_initialBox;
		math::Vector3 elongation = static_cast<math::Vector3>(m_box.Diagonal()) * (MAX_STRETCHING_RATIO - 1.f) ;

		if (octetIndex % 2 == 0)
		{
			m_box.Center.x  += elongation.x * 0.5f;
			m_box.Extents.x += elongation.x * 0.5f;
		}
		else 
		{
			m_box.Center.x  -= elongation.x * 0.5f;
			m_box.Extents.x -= elongation.x * 0.5f;
		}

		if (octetIndex % 4 < 2) 
		{
			m_box.Center.y  += elongation.y * 0.5f;
			m_box.Extents.y += elongation.y * 0.5f;

		}
		else 
		{
			m_box.Center.y  -= elongation.y * 0.5f;
			m_box.Extents.y -= elongation.y * 0.5f;
		}

		if (octetIndex < 4) 
		{
			m_box.Center.z  += elongation.z * 0.5f;
			m_box.Extents.z += elongation.z * 0.5f;
		}
		else 
		{
			m_box.Center.z  -= elongation.z * 0.5f;
			m_box.Extents.z -= elongation.z * 0.5f;
		}
	}

	bool TriangleOctree::addTriangle(uint32_t triangleIndex, const math::Vector3& V1, const math::Vector3& V2, const math::Vector3& V3, const math::Vector3& center)
	{
		bool centerContained{ m_initialBox.Contains(center) == DirectX::ContainmentType::CONTAINS };
		bool v1Contained{ m_box.Contains(V1) == DirectX::ContainmentType::CONTAINS };
		bool v2Contained{ m_box.Contains(V2) == DirectX::ContainmentType::CONTAINS };
		bool v3Contained{ m_box.Contains(V3) == DirectX::ContainmentType::CONTAINS };

		if (!centerContained ||
			!v1Contained     ||
			!v2Contained     ||
			!v3Contained)
		{
			return false;
		}

		if (m_children == nullptr)
		{
			if (m_triangles.size() < PREFFERED_TRIANGLE_COUNT)
			{
				m_triangles.emplace_back(triangleIndex);
				return true;
			}
			else
			{
				math::Vector3 C = static_cast<math::Vector3>(m_initialBox.Center) / 2.f;

				m_children.reset(new std::array<TriangleOctree, 8>());
				for (int i = 0; i < 8; ++i)
				{
					(*m_children)[i].initialize(*m_mesh, m_initialBox, C, i);
				}

				std::vector<uint32_t> newTriangles;

				for (uint32_t index : m_triangles)
				{
					const math::Vector3 P1 = getPos(*m_mesh, index, 0);
					const math::Vector3 P2 = getPos(*m_mesh, index, 1);
					const math::Vector3 P3 = getPos(*m_mesh, index, 2);

					math::Vector3 P = (P1 + P2 + P3) / 3.f;

					int i = 0;
					for (; i < 8; ++i)
					{
						if ((*m_children)[i].addTriangle(index, P1, P2, P3, P))
							break;
					}

					if (i == 8)
						newTriangles.emplace_back(index);
				}

				m_triangles = std::move(newTriangles);
			}
		}

		int i = 0;
		for (; i < 8; ++i)
		{
			if ((*m_children)[i].addTriangle(triangleIndex, V1, V2, V3, center))
				break;
		}

		if (i == 8)
			m_triangles.emplace_back(triangleIndex);

		return true;
	}

	bool TriangleOctree::intersect(const Ray& ray, MeshIntersection& nearest) const
	{
		return intersectInternal(ray, nearest);
	}

	bool TriangleOctree::intersectInternal(const Ray& ray, MeshIntersection& nearest) const
	{
		{
			float boxT = nearest.t;
			if (!m_box.Intersects(ray.position, ray.direction, boxT))
				return false;
		}

		bool found = false;
		float t{};
		for (uint32_t i = 0; i < m_triangles.size(); ++i)
		{
			const math::Vector3 V1 = getPos(*m_mesh, m_triangles[i], 0);
			const math::Vector3 V2 = getPos(*m_mesh, m_triangles[i], 1);
			const math::Vector3 V3 = getPos(*m_mesh, m_triangles[i], 2);
			
			
			if (DirectX::TriangleTests::Intersects(ray.position, ray.direction, V1, V2, V3, t) && t < nearest.t)
			{
				//Move this calculations to MeshInstance.Intersects()
				nearest.t = t;
				math::Vector3 normal{ V1.Cross(V2) };
				normal.Normalize();
				nearest.normal = normal;
				nearest.pos = ray.at(t);
				nearest.triangle = i;
				found = true;
			}
		}

		if (!m_children) return found;

		struct OctantIntersection
		{
			int index;
			float t;
		};

		std::array<OctantIntersection, 8> boxIntersections;

		for (int i = 0; i < 8; ++i)
		{
			if ((*m_children)[i].m_box.Contains(ray.position))
			{
				boxIntersections[i].index = i;
				boxIntersections[i].t = 0.f;
			}
			else
			{
				float boxT = nearest.t;
				/*if (ray.intersect(boxT, (*m_children)[i].m_box, true))
				{
					boxIntersections[i].index = i;
					boxIntersections[i].t = boxT;
				}*/
				
				if ((*m_children)[i].m_box.Intersects(ray.position, ray.direction, boxT))
				{
					boxIntersections[i].index = i;
					boxIntersections[i].t = boxT;
				}
				else
				{
					boxIntersections[i].index = -1;
				}
			}
		}

		std::sort(boxIntersections.begin(), boxIntersections.end(),
			[](const OctantIntersection& A, const OctantIntersection& B) -> bool { return A.t < B.t; });

		for (int i = 0; i < 8; ++i)
		{
			if (boxIntersections[i].index < 0 || boxIntersections[i].t > nearest.t)
				continue;

			if ((*m_children)[boxIntersections[i].index].intersectInternal(ray, nearest))
			{
				found = true;
			}
		}

		return found;
	}

}