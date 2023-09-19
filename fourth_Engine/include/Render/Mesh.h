#pragma once
#include "include/Render/Primitive Shapes/Vertex.h"
#include "include/Render/TriangleOctree.h"


namespace fth
{
	struct Ray;
	class Model;


	class Mesh
	{
	public:
		struct MeshRange
		{
			uint32_t     vertexOffset;
			uint32_t     indexOffset;
			uint32_t     triangleOffset;
			uint32_t     vertexNum;
			uint32_t     indexNum;
			uint32_t     triangleNum;
		};

		Mesh();
		~Mesh();
		Mesh(const uint32_t& numVertices);
		Mesh(const Mesh& otherMesh) = delete;
		Mesh& operator=(const Mesh& otherMesh) = delete;
		Mesh(Mesh&& otherMesh);


		void Initialize(std::shared_ptr<Model> srcModel);
		void Shutdown();

		inline const math::AABB& GetAABB() const { return m_AABB; }
		uint32_t GetNumOfTriangles() const;
		const TriangleIndexed& GetTriangle(uint32_t index) const;
		const Vertex& GetVertexFromIndexedTriangle(uint32_t triangleIndex, uint32_t triangleVertexIndex) const;
		const Vertex& GetVertex(uint32_t vertexIndex) const;

		bool IntersectsAABB(const Ray& ray, float& tDistance) const;
		bool Intersects(const Ray& ray, MeshIntersection& outIntersection) const;

	
	public:
		std::shared_ptr<Model>              m_sourceModel;
		MeshRange                           m_meshRange;
		std::vector<math::Matrix>           m_nodeParentTransforms;
		std::vector<math::Matrix>           m_parentNodeTransforms;
		math::Matrix                        m_meshToModel = math::Matrix::Identity;
		math::Matrix                        m_modelToMesh = math::Matrix::Identity;
		math::AABB                          m_AABB;
		std::string                         m_meshName;
		TriangleOctree                      m_TriangleOctree;


	};
}