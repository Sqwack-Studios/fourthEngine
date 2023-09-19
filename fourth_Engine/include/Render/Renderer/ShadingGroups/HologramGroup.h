#pragma once
#include "include/Render/Renderer/ShadingGroups/ShadingGroup.h"
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/ShadingGroups/HullShader.h"
#include "include/Render/Renderer/ShadingGroups/DomainShader.h"
#include "include/Render/Renderer/ShadingGroups/GeometryShader.h"
#include "include/Render/Renderer/UniformBuffer.h"
#include "include/Render/Renderer/InstanceBuffer.h"
#include "include/Systems/TransformSystem.h"

namespace fth
{
	namespace shading
	{
		class HologramGroup: public IShadingGroup
		{
		public:
			struct PerModel;
			struct InstanceData;
			struct Material;

			virtual void Init(uint8_t modelBits, uint8_t materialBits, uint8_t instanceBits) override;
			virtual void Render() override;
			virtual void RenderDebugNormals() override;
			virtual void UpdateInstanceBuffer() override;
			bool FindIntersection(const Ray& ray, math::HitResult& outHit, InstanceFinder& outInstanceFinder);
			//virtual void GetInstanceTransforms(const InstanceFinder& intersectionIdx, std::vector<TransformID>& outInstances) override;

			Handle<PerModel> addModel(const std::shared_ptr<Model>& srcModel);
			void addMaterial(const Material* materials, Handle<PerModel> model, Handle<Material>* outHandles);
			void addInstances(Handle<PerModel> model, const Handle<Material>* materials , const InstanceData* data, uint32_t numInstances, PerMeshInstance* outIDs);

		private:

			void RenderInternal();
			struct InstanceDataGPU
			{
				math::Matrix   modelToWorld;
				math::Vector3  color;
			};

		public:
			struct InstanceData
			{
				Handle<math::Matrix>          handle_modelToWorld;
				Handle<RenderableObject>      handle_object;
				Handle<ModelInstance>         handle_modelInstance;
				math::Vector3 color;
			};
			struct PerMeshData
			{
				math::Matrix meshToModel;
			};

			struct Material
			{
				bool operator==(const Material&) { return true; }
			};
			struct PerMaterial
			{
				Material material;
				SolidVector<InstanceData> instances;
			};
			struct PerMesh
			{
				std::vector<PerMaterial> perMaterial;
			};

			struct PerModel
			{
				PerModel(const std::shared_ptr<Model>& srcModel);
				bool operator==(const std::shared_ptr<Model>& rvalue)
				{
					return srcModel.get() == rvalue.get();
				}
				std::shared_ptr<Model> srcModel;
				std::vector<PerMesh>   perMesh;
			};
		protected:
			friend bool utility::findIntersection(HologramGroup&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
			friend bool utility::findIntersectionInternal(HologramGroup&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
			friend void utility::addModel(HologramGroup&, const std::shared_ptr<Model>&, Handle<PerModel>&);
			friend void utility::addMaterial(HologramGroup&, const Material*, Handle<PerModel>, Handle<Material>*);
			friend void utility::addInstances(HologramGroup&, Handle<PerModel>, const Handle<Material>*, const InstanceData*, uint32_t, PerMeshInstance*);


			renderer::VertexShader                     m_vs;
			renderer::PixelShader                      m_ps;
			renderer::HullShader                       m_hs;
			renderer::DomainShader                     m_ds;
			renderer::GeometryShader                   m_gs;

			//Normal debugging
			renderer::GeometryShader                   m_normalDebugGS;

			renderer::InstanceBuffer                   m_instanceBuffer;
			renderer::UniformBuffer                    m_perMeshUniform;
			//

			std::vector<PerModel>                      m_perModel;
			uint32_t                                   m_numInstances = 0;
		};
	}
}