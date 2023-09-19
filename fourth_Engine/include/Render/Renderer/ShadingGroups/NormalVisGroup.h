#pragma once
#include "include/Render/Renderer/ShadingGroups/ShadingGroup.h"
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/ShadingGroups/GeometryShader.h"
#include "include/Render/Renderer/UniformBuffer.h"
#include "include/Render/Renderer/InstanceBuffer.h"
#include "include/Systems/TransformSystem.h"


namespace fth
{

	namespace shading
	{

		class NormalVisGroup : public IShadingGroup
		{
		public:
			//Exposed to public interface and what is sent to the gpu/per instance
			struct PerModel;
			struct Material;

			struct InstanceData
			{
				//InstanceData(TransformID transformID, ObjectID objectID) : 
				//	id_modelToWorld(transformID), id_object(objectID) {}

				Handle<math::Matrix>        handle_modelToWorld;
				Handle<ModelInstance>       handle_modelInstance;
				Handle<RenderableObject>    handle_object;
			};


			virtual void Init(uint8_t modelBits, uint8_t materialBits, uint8_t instanceBits) override;
			virtual void UpdateInstanceBuffer() override;
			virtual void Render() override;
			virtual void RenderDebugNormals() override;
			//virtual void GetInstanceTransforms(const InstanceFinder& intersectionIdx, std::vector<TransformID>& outInstances) override;
			bool FindIntersection(const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outInstanceFinder);

			Handle<PerModel> addModel(const std::shared_ptr<Model>& srcModel);
			void addMaterial(const Material* materials, Handle<PerModel> model, Handle<Material>* outHandles);
			void addInstances(Handle<PerModel> model, const Handle<Material>* materials, const InstanceData* data, uint32_t numInstances, PerMeshInstance* outIDs);


		private:
			void RenderInternal();

		private:
			struct InstanceDataGPU
			{
				math::Matrix modelToWorld;
			};
		public:
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
				bool operator==(const PerModel& rval)
				{
					return srcModel == rval.srcModel;
				}
				std::shared_ptr<Model> srcModel;
				std::vector<PerMesh>   perMesh;
			};


		private:
			friend bool utility::findIntersection(NormalVisGroup&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
			friend bool utility::findIntersectionInternal(NormalVisGroup&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
			friend void utility::addModel(NormalVisGroup&, const std::shared_ptr<Model>&, Handle<PerModel>&);
			friend void utility::addMaterial(NormalVisGroup&, const Material*, Handle<PerModel>, Handle<Material>*);
			friend void utility::addInstances(NormalVisGroup&, Handle<PerModel>, const Handle<Material>*, const InstanceData*, uint32_t numInstances, PerMeshInstance*);

			renderer::VertexShader                 m_vs;
			renderer::PixelShader                  m_ps;

			//Normal debugging
			renderer::GeometryShader               m_debugNormalsGS;

			std::vector<PerModel>                  m_perModel;
			renderer::UniformBuffer                m_perMeshUniform;
			renderer::InstanceBuffer               m_instanceBuffer;
			uint32_t                               m_numInstances = 0;

		};
	}
}