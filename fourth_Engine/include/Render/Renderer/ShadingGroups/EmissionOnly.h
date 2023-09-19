#pragma once
#include "include/Render/Renderer/ShadingGroups/ShadingGroup.h"
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"
#include "include/Render/Renderer/ShadingGroups/GeometryShader.h"
#include "include/Render/Renderer/InstanceBuffer.h"
#include "include/Render/Renderer/UniformBuffer.h"
#include "include/Render/Renderer/ConstantBuffers.h"
#include "include/Systems/TransformSystem.h"


namespace fth::shading
{
	//Maybe group per light types. Also maybe create a base light class which contains common data like transforms ID and color. This could avoid grouping by types
	class EmissionOnly: public IShadingGroup
	{
	public:
		struct PerModel;
		struct Material;
		struct InstanceData;

		virtual void Init(uint8_t modelBits, uint8_t materialBits, uint8_t instanceBits) override;
		virtual void UpdateInstanceBuffer() override;
		virtual void Render() override;
		virtual void RenderDebugNormals() override;

		Handle<EmissionOnly::PerModel> addModel(const std::shared_ptr<Model>& srcModel);
		void addMaterial(const Material* materials, Handle<PerModel> modelIdx, Handle<Material>* outHandles);
		void addInstances(Handle<PerModel> modelIdx, const Handle<Material>* matIdx, const InstanceData* data, uint32_t numInstances, PerMeshInstance* outIDs);


		//virtual void GetInstanceTransforms(const InstanceFinder& intersectionIdx, std::vector<TransformID>& outInstances);
		bool FindIntersection(const Ray& ray, math::HitResult& outHit, InstanceFinder& outInstanceFinder);


	private:
		void RenderInternal();

		struct InstanceDataGPU
		{
			math::Matrix  modelToWorld;
			math::Vector3 emissionColor;
			float         pad[1];
		};
	public:
		struct InstanceData
		{
			Handle<math::Matrix>          handle_modelToWorld;
			Handle<ModelInstance>         handle_modelInstance;
			Handle<RenderableObject>      handle_object;
			math::Vector3 emissionColor;
		};

		struct Material
		{
			bool operator ==(const Material&) { return true; }
		};

		struct PerMeshData
		{
			math::Matrix meshToModel;
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
		friend bool utility::findIntersection(EmissionOnly&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
		friend bool utility::findIntersectionInternal(EmissionOnly&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
		friend void utility::addModel(EmissionOnly&, const std::shared_ptr<Model>& srcModel, Handle<EmissionOnly::PerModel>&);
		friend void utility::addMaterial(EmissionOnly&, const Material*, Handle<PerModel>, Handle<Material>*);
		friend void utility::addInstances(EmissionOnly&, Handle<PerModel>, const Handle<Material>*, const InstanceData*, uint32_t, PerMeshInstance*);

		renderer::PixelShader                          m_PS;
		renderer::VertexShader                         m_VS;
		renderer::GeometryShader                       m_normalDebugGS;

		renderer::UniformBuffer	                       m_meshUniform;
		renderer::InstanceBuffer                       m_instanceData;
		std::vector<PerModel>                          m_perModel;

		uint32_t                                       m_numInstances = 0;
    };
}