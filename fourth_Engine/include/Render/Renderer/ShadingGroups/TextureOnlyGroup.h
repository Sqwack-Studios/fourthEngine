#pragma once
#include "include/Render/Renderer/ShadingGroups/ShadingGroup.h"
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"
#include "include/Render/Renderer/ShadingGroups/GeometryShader.h"
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/VertexBuffer.h"
#include "include/Render/Renderer/InstanceBuffer.h"
#include "include/Render/Renderer/UniformBuffer.h"
#include "include/Systems/TransformSystem.h"

namespace fth
{
	class Texture;
}

namespace fth::shading
{
	class TextureOnlyGroup : public IShadingGroup
	{
	public:
		struct PerModel;
		struct PerMesh;
		struct Material;
		struct InstanceData;
		struct PerMaterial;


		virtual void Init(uint8_t modelBits, uint8_t materialBits, uint8_t instanceBits) override;
		virtual void UpdateInstanceBuffer() override;
		virtual void Render() override;
		virtual void RenderDebugNormals() override;

		bool FindIntersection(const Ray& ray, math::HitResult& outHit, InstanceFinder& outFinder);
		//virtual void GetInstanceTransforms(const  InstanceFinder& intersectionIdx, std::vector<TransformID>& outInstances ) override;

		Handle<PerModel> addModel(const std::shared_ptr<Model>& srcModel);
		void addMaterial(const Material* materials, Handle<PerModel> model, Handle<Material>* outHandles);
		void addInstances(Handle<PerModel> model, const Handle<Material>* materials, const InstanceData* data, uint32_t numInstances, PerMeshInstance* outIDs);

	private:

		struct InstanceDataGPU
		{
			math::Matrix  modelToWorld;
		};
		void RenderInternal();


	public:

		struct InstanceData
		{
			Handle<math::Matrix>         handle_modelToWorld;
			Handle<RenderableObject>     handle_object;
			Handle<ModelInstance>        handle_modelInstance;
		};

		struct PerMeshData
		{
			math::Matrix meshToModel;
		};

		//Things to upload to the gpu if we want
		struct PerMaterialData
		{
			float   uScale = 1.0f;
			float   vScale = 1.0f;
			float   pad[2];
		};

		struct Material
		{
			void Bind() const;
			bool operator==(const Material& rvalue) { return albedo == rvalue.albedo; }

			std::shared_ptr<Texture> albedo;
		};

		struct PerMaterial
		{
			Material                              material;     //Bindable to gpu
			//PerMaterialData                     materialData;         //Upload to gpu-> right now affects all mesh instances, to make it per instance, create a vector
			SolidVector<InstanceData>     instances;
		};

		struct PerMesh
		{
			std::vector<PerMaterial> perMaterial;
		};

		struct PerModel
		{
			PerModel(std::shared_ptr<Model> model);

			bool operator==(const PerModel& rval) { return srcModel == rval.srcModel; }
			bool empty() const { return perMesh.size() == 0; }

			std::shared_ptr<Model>           srcModel;
			std::vector<PerMesh>             perMesh;
		};
	private:
		friend bool utility::findIntersection(TextureOnlyGroup&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
		friend bool utility::findIntersectionInternal(TextureOnlyGroup&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
		friend void utility::addModel(TextureOnlyGroup&, const std::shared_ptr<Model>&, Handle<TextureOnlyGroup::PerModel>&);
		friend void utility::addMaterial(TextureOnlyGroup&, const Material*, Handle<PerModel>, Handle<Material>*);
		friend void utility::addInstances(TextureOnlyGroup&, Handle<PerModel>, const Handle<Material>*, const InstanceData*, uint32_t, PerMeshInstance*);

		renderer::VertexShader                              m_vs;
		renderer::PixelShader                               m_ps;

		//Normal Debugging
		renderer::VertexShader                              m_normalDebugVS;
		renderer::GeometryShader                            m_debugGS;


		std::vector<PerModel>                               m_perModel;
		renderer::InstanceBuffer                            m_instanceBuffer;
		renderer::UniformBuffer                             m_meshUniform;
		renderer::UniformBuffer                             m_materialUniform;
		uint32_t                                            m_numInstances = 0;
	};

}
