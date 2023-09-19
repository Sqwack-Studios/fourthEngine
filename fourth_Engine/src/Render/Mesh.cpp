#include "pch.h"

#pragma once
#include "include/Render/Mesh.h"
#include "include/Math/Ray.h"
#include "include/Render/Model.h"
#include "include/Render/Renderer/D3D11Headers.h"


namespace fth
{

	Mesh::Mesh(const uint32_t& numVertices):
		m_meshToModel(math::Matrix::Identity),
		m_modelToMesh(math::Matrix::Identity)
	{

	}

	Mesh::Mesh(Mesh&& otherMesh):
		m_TriangleOctree(std::move(otherMesh.m_TriangleOctree)),
		m_AABB(otherMesh.m_AABB)
	{

	}

	Mesh::Mesh() {}
	Mesh::~Mesh()
	{
		m_TriangleOctree.clear();
	}

	void Mesh::Initialize(std::shared_ptr<Model> srcModel)
	{
		m_sourceModel = srcModel;
		m_TriangleOctree.initialize(*this);
	}

	void Mesh::Shutdown()
	{
		m_TriangleOctree.clear();
	}

	uint32_t Mesh::GetNumOfTriangles() const
	{
		return m_meshRange.triangleNum;
	}

	const TriangleIndexed& Mesh::GetTriangle(uint32_t index) const
	{
		return  m_sourceModel->GetTriangleIndexBuffer()[m_meshRange.triangleOffset + index];
	}

	const Vertex& Mesh::GetVertexFromIndexedTriangle(uint32_t triangleIndex, uint32_t triangleVertexIndex) const
	{
		return GetVertex(GetTriangle(triangleIndex).indexes[triangleVertexIndex]);
	}

	const Vertex& Mesh::GetVertex(uint32_t vertexIndex) const
	{
		return m_sourceModel->GetVertexBuffer()[m_meshRange.vertexOffset + vertexIndex];
	}

	bool Mesh::IntersectsAABB(const Ray& inRay, float& tDistance) const
	{
		//transform to mesh space
		Ray ray{ math::TransformRay(inRay, m_modelToMesh) };


		float t{std::numeric_limits<float>::infinity()};
		bool foundIntersection{ m_AABB.Intersects(ray.position, ray.direction, t) };
		if (foundIntersection)
		{
			//transform back to model space
			ray = math::TransformRay(ray, m_meshToModel);
			tDistance = (ray.position - ray.at(t)).Length();
		}

		return foundIntersection;
	}

	bool Mesh::Intersects(const Ray& inRay, MeshIntersection& outIntersection) const
	{
		MeshIntersection meshInter;
		meshInter.reset(0.0f);
		Ray ray{ math::TransformRay(inRay, m_modelToMesh) };
		

		bool foundIntersection{ m_TriangleOctree.intersect(ray, meshInter) };
		if (foundIntersection)
		{
			math::Vector3 hitPos{ math::TransformPoint(meshInter.pos, m_meshToModel) };
			float f{ (inRay.position - hitPos).Length() };
			if (outIntersection.t > f)
			{
				outIntersection.pos = hitPos;
				outIntersection.normal = math::TransformVector(meshInter.normal, m_meshToModel);
				outIntersection.normal.Normalize();
				outIntersection.t = f;
			}
		}

		return foundIntersection;
	}
}