#pragma once
#include "include/Render/Renderer/VertexBuffer.h"
#include "include/Render/Renderer/IndexBuffer.h"
#include "include/Render/Mesh.h"


namespace fth
{
	class ModelManager;
}
namespace fth
{
	class Model
	{
	public:
		Model(std::string_view name);
		Model();
		~Model();
		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		
		Model(Model&& other) = default;
		Model& operator=(Model&& other) = default;
	

		void Init(std::shared_ptr<Model> selfPtr);
		void Shutdown();
		
		const std::vector<Vertex>& GetVertexBuffer() { return m_vertices.GetCpuData(); }
		const std::vector<TriangleIndexed>& GetTriangleIndexBuffer() { return m_trianglesIndexed.GetCpuData(); }
		uint32_t GetNumMeshes() const { return static_cast<uint32_t>(m_meshes.size()); }
		const std::string& GetName() const { return m_name; }
		const std::vector<Mesh>& GetMeshes() const { return m_meshes; }
		inline void BindVertexBuffer() const { m_vertices.Bind(0) ; };
		bool BindIndexBuffer() const;

		const math::AABB& getAABB() const { return m_aabb; }
	private:
		friend class ModelManager;

		std::string                                     m_name;
		std::vector<Mesh>                               m_meshes;
		renderer::VertexBuffer<Vertex>                  m_vertices;
		renderer::IndexBuffer<TriangleIndexed>          m_trianglesIndexed;
		math::AABB                                      m_aabb;


	};
}