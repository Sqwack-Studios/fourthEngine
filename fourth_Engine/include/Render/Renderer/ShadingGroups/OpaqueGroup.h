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
}


namespace fth::shading
{
	class OpaqueGroup: public IShadingGroup
	{
	public:
		struct PerModel;
		struct Material;
		struct InstanceData;
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
		virtual void UpdateInstanceBuffer() override;
		virtual void Render() override;
		virtual void RenderDebugNormals() override;
		//virtual void GetInstanceTransforms(const InstanceFinder& intersectionIdx, std::vector<TransformID>& outInstances) override;

		void RenderDepth2DOnly();
		void RenderDepthCubemapsOnly(const math::Matrix* vp, uint16_t numCubemaps);

		void SetDepthPass_2D(const renderer::VertexShader&shader);
		void SetDepthPass_Cube(const renderer::VertexShader& vs, const renderer::GeometryShader& gs);

		Handle<OpaqueGroup::PerModel> addModel(const std::shared_ptr<Model>& srcModel);
		void addMaterial(const Material* materials, Handle<PerModel> model, Handle<Material>* outHandles);
		void addInstances(Handle<PerModel> model, const Handle<Material>* materials, const InstanceData* data, uint32_t numInstances, PerMeshInstance* outIDs);
		bool queryInstanceMaterial(const Handle<CombinedID>* meshInstances, uint32_t numMeshes, Material* outMaterials) const;
		const std::shared_ptr<Model>& getModelByHandle(Handle<CombinedID> modelHandle) const;
		bool FindIntersection(const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
		bool deleteInstance(const Handle<CombinedID>* meshInstances, uint32_t numMeshInstances);


		struct Material
		{
			Material() = default;
			Material(
				const std::shared_ptr<Texture>& albedo,
				const std::shared_ptr<Texture>& normal,
				const std::shared_ptr<Texture>& roughness,
				const std::shared_ptr<Texture>& metalness,
				uint16_t flags,
				float f_roughness,
				float f_metalness);

			void Bind() const;
			bool operator==(const Material& rvalue) const { return albedo == rvalue.albedo && normal == rvalue.normal && roughness == rvalue.roughness && metalness == rvalue.metalness && flags == rvalue.flags; }

			std::shared_ptr<Texture> albedo   ;
			std::shared_ptr<Texture> normal   ;
			std::shared_ptr<Texture> roughness;
			std::shared_ptr<Texture> metalness;

			uint16_t flags;

			float f_roughness = 1.0f;
			float f_metalness = 0.0f;
		};

	private:

		void RenderInternal();
	private:
		//SEND TO GPU
		struct InstanceDataGPU
		{
			math::Matrix modelToWorld;
			uint32_t     id_object;
		};
		struct PerMeshDataGPU
		{
			math::Matrix meshToModel;
		};

		struct PerMaterialGPU
		{
			uint32_t  flags;
			float     roughness;
			float     metalness;
			float     pad[1];
		};

	public:

		struct InstanceData
		{
			Handle<math::Matrix>      handle_modelToWorld;
			Handle<RenderableObject>  handle_object;
			Handle<ModelInstance>     handle_modelInstance;
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
		friend bool utility::findIntersection(OpaqueGroup&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
		friend bool utility::findIntersectionInternal(OpaqueGroup&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
		friend void utility::addModel(OpaqueGroup&, const std::shared_ptr<Model>&, Handle<PerModel>& outHandle);
		friend void utility::addMaterial(OpaqueGroup&, const Material*, Handle<PerModel>, Handle<Material>*);
		friend void utility::addInstances(OpaqueGroup&, Handle<PerModel>, const Handle<Material>*, const InstanceData*, uint32_t, PerMeshInstance*);
		friend bool utility::queryInstanceMaterial(const OpaqueGroup&, const Handle<CombinedID>*, uint32_t, Material*);
		friend const std::shared_ptr<Model>& utility::getModelByHandle(const OpaqueGroup& group, Handle<CombinedID> modelHandle);
		friend bool utility::deleteInstance(OpaqueGroup&, const Handle<CombinedID>*, uint32_t);

		renderer::PixelShader                         m_PS;
		renderer::VertexShader                        m_VS;
		renderer::GeometryShader                      m_normalDebugGS;

		//Depth
		renderer::VertexShader                        m_depthPass2D_VS;
		renderer::VertexShader                        m_depthPassCube_VS;
		renderer::GeometryShader                      m_depthPassCube_GS;
		renderer::UniformBuffer                       m_depthPassCube_Uniform;

										                  
		renderer::UniformBuffer                       m_perMeshUniform;
		renderer::UniformBuffer                       m_perMaterialUniform;
		renderer::InstanceBuffer                      m_instanceBuffer;
		std::vector<PerModel>                         m_perModel;


		uint32_t                                      m_numInstances;
	};
}