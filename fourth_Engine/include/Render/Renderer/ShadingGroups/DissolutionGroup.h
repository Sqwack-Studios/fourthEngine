#pragma once
#include "include/Render/Renderer/ShadingGroups/ShadingGroup.h"
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"
#include "include/Render/Renderer/ShadingGroups/GeometryShader.h"
#include "include/Render/Renderer/UniformBuffer.h"
#include "include/Render/Renderer/InstanceBuffer.h"
#include "include/Systems/TransformSystem.h"

namespace fth
{
	class Texture;
	struct TexturePack;
}

namespace fth::shading
{
	
	class DissolutionGroup : public IShadingGroup
	{
	public:
		struct InstanceData;
		struct PerModel;
		struct Material;
		struct PerMesh;

		enum MaterialFlags
		{
			//CHECKED BY THE CONSTRUCTOR
			HAS_NORMALS = 1,
			HAS_ROUGHNESS = 2,
			HAS_METALNESS = 4,
			//CHECKED BY THE END USER
			INVERT_Y_CHANNEL = 8,
			BUILD_BLUE_CHANNEL = 16
		};

		virtual void Init(uint8_t modelBits, uint8_t materialBits, uint8_t instanceBits) override;
		void update(float deltaTime);
		virtual void UpdateInstanceBuffer() override;


		virtual void Render() override;
		virtual void RenderDebugNormals() override;

		Handle<DissolutionGroup::PerModel> addModel(const std::shared_ptr<Model>& srcModel);
		void addMaterial(const Material* materials, Handle<PerModel> model, Handle<Material>* outHandles);
		void addInstances(Handle<PerModel> model, const Handle<Material>* materials, const InstanceData* data, uint32_t numInstances, PerMeshInstance* outIDs);

		const InstanceData& getInstanceData(PerMeshInstance targetInstance) const;
		InstanceData& getInstanceData(PerMeshInstance targetInstance) 
		{ 
			return const_cast<InstanceData&>(
				   static_cast<const DissolutionGroup&>(*this).getInstanceData(targetInstance)
					);
		}

		bool deleteInstance(const Handle<CombinedID>* instanceHandles, uint32_t numMeshes);
		void RenderDepth2DOnly();
		void RenderDepthCubemapsOnly(const math::Matrix* vp, uint16_t numCubemaps);

		bool FindIntersection(const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);

		const std::shared_ptr<Model>& getModelByHandle(Handle<CombinedID> handle) const;

		struct InstanceData
		{
			Handle<math::Matrix>        handle_modelToWorld;
			Handle<RenderableObject>    handle_object;
			Handle<ModelInstance>       handle_modelInstance;
			float                       animationDuration;
			float                       animationTime;

		};

	private:
		void RenderInternal();

	private:
		//SEND TO GPU
		struct InstanceDataGPU
		{
			math::Matrix modelToWorld;
			uint32_t     id_object;
			float        animationDuration;
			float        animationTime;
		};
		struct PerMeshDataGPU
		{
			math::Matrix meshToModel;
		};

		struct PerMaterialGPU
		{
			uint32_t  flags;
			float     f_roughness;
			float     f_metalness;

			float     pad[1];
		};
	public:
		struct Material
		{
			Material() = default;
			Material(
				const std::shared_ptr<Texture>& albedo,
				const std::shared_ptr<Texture>& normal,
				const std::shared_ptr<Texture>& roughness,
				const std::shared_ptr<Texture>& metalness,
				const std::shared_ptr<Texture>& noise,
				uint16_t flags,
				float f_roughness,
				float f_metalness);

			void Bind() const;
			bool operator==(const Material& rvalue) { return albedo == rvalue.albedo && normal == rvalue.normal && roughness == rvalue.roughness && metalness == rvalue.metalness && dissolutionNoise == rvalue.dissolutionNoise; }

			std::shared_ptr<Texture> albedo;
			std::shared_ptr<Texture> normal;
			std::shared_ptr<Texture> roughness;
			std::shared_ptr<Texture> metalness;
			std::shared_ptr<Texture> dissolutionNoise;

			uint32_t flags;
			float    f_roughness = 1.0f;
			float    f_metalness = 0.0f;
		};

		struct PerMaterial
		{
			Material   material;
			SolidVector<InstanceData> instances;
		};
		struct PerMesh
		{
			std::vector<PerMaterial> perMaterial;
		};
		struct PerModel
		{
			PerModel(const std::shared_ptr<Model>& srcModel);
			bool operator==(const PerModel& rval) { return srcModel == rval.srcModel; }

			std::shared_ptr<Model>   srcModel;
			std::vector<PerMesh> perMesh;
		};

	private:

		friend bool utility::findIntersection(DissolutionGroup&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
		friend bool utility::findIntersectionInternal(DissolutionGroup&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
		friend void utility::addModel(DissolutionGroup&, const std::shared_ptr<Model>&, Handle<PerModel>& outHandle);
		friend void utility::addMaterial(DissolutionGroup&, const Material*, Handle<PerModel>, Handle<Material>*);
		friend void utility::addInstances(DissolutionGroup&, Handle<PerModel>, const Handle<Material>*, const InstanceData*, uint32_t, PerMeshInstance*);
		friend const DissolutionGroup::InstanceData& utility::getInstanceData(const DissolutionGroup&, PerMeshInstance targetInstance);
		friend bool utility::deleteInstance(DissolutionGroup&, const Handle<CombinedID>*, uint32_t);
		friend const std::shared_ptr<Model>& utility::getModelByHandle(const DissolutionGroup&, Handle<CombinedID>);


		renderer::PixelShader                         m_PS;
		renderer::VertexShader                        m_VS;
		renderer::GeometryShader                      m_normalDebugGS;

		//Depth
		renderer::VertexShader                        m_depthPass2D_VS;
		renderer::PixelShader                         m_depthPass2D_PS;

		renderer::VertexShader                        m_depthPassCube_VS;
		renderer::GeometryShader                      m_depthPassCube_GS;
		renderer::PixelShader                         m_depthPassCube_PS;
		renderer::UniformBuffer                       m_depthPassCube_Uniform;


		renderer::UniformBuffer                       m_perMeshUniform;
		renderer::UniformBuffer                       m_perMaterialUniform;
		renderer::InstanceBuffer                      m_instanceBuffer;
		std::vector<PerModel>                         m_perModel;

		uint32_t                                      m_numInstances;
	};
}