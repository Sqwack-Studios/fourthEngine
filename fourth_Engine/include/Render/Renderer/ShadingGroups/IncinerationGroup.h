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
	class IncinerationGroup: public IShadingGroup
	{
	public:
		struct PerModel;
		struct Material;
		struct InstanceData;

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
		void RenderDepth2DOnly();
		void RenderDepthCubemapsOnly(const math::Matrix* vp, uint16_t numCubemaps);

		void update(float deltaTime);
		
		Handle<IncinerationGroup::PerModel> addModel(const std::shared_ptr<Model>& srcModel);
		void addMaterial(const Material* materials, Handle<PerModel> modelIdx, Handle<Material>* outHandles);
		void addInstances(Handle<PerModel> modelIdx, const Handle<Material>* matIdx, const InstanceData* data, uint32_t numInstances, PerMeshInstance* outIDs);

		const InstanceData& getInstanceData(PerMeshInstance targetInstance) const;
		InstanceData& getInstanceData(PerMeshInstance targetInstance)
		{
			return const_cast<InstanceData&>(
				static_cast<const IncinerationGroup&>(*this).getInstanceData(targetInstance)
				);

		}
		bool deleteInstance(const Handle<CombinedID>* meshInstances, uint32_t numMeshInstances);

		bool FindIntersection(const Ray& ray, math::HitResult& outHit, InstanceFinder& outInstanceFinder);
	private:
		void renderInternal();

		friend void utility::addModel(IncinerationGroup&, const std::shared_ptr<Model>& srcModel, Handle<IncinerationGroup::PerModel>&);
		friend bool utility::findIntersection(IncinerationGroup&, const Ray&, math::HitResult&, InstanceFinder&);
		friend bool utility::findIntersectionInternal(IncinerationGroup&, const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder);
		friend void utility::addMaterial(IncinerationGroup&, const Material*, Handle<PerModel>, Handle<Material>*);
		friend void utility::addInstances(IncinerationGroup&, Handle<PerModel>, const Handle<Material>*, const InstanceData*, uint32_t numInstances, PerMeshInstance*);
		friend bool utility::deleteInstance(IncinerationGroup&, const Handle<CombinedID>*, uint32_t numMeshInstances);
		friend const InstanceData& utility::getInstanceData(const IncinerationGroup&, PerMeshInstance);

	public:
		struct InstanceData
		{
			math::Vector3              sphereToModelPos;
			float                      radius;

			math::Vector3              emission;
			float                      lastFrameRadius;

			float                      radiusGrowthRate;
			Handle<math::Matrix>       handle_modelToWorld;
			Handle<ModelInstance>      handle_modelInstance;
			Handle<RenderableObject>   handle_object;

		};

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
			bool operator==(const Material& rvalue) const { return albedo == rvalue.albedo && normal == rvalue.normal && roughness == rvalue.roughness && metalness == rvalue.metalness && flags == rvalue.flags; }

			std::shared_ptr<Texture> albedo;
			std::shared_ptr<Texture> normal;
			std::shared_ptr<Texture> roughness;
			std::shared_ptr<Texture> metalness;
			std::shared_ptr<Texture> noise;
			uint16_t flags;

			float f_roughness = 1.0f;
			float f_metalness = 0.0f;
		};


	private:
		struct InstanceDataGPU
		{
			math::Matrix modelToWorld;
			math::Vector3 color;
			math::Vector3 sphereToModelPos;
			uint32_t      objectID;
			float currentRadiusSq;
			float prevRadiusSq;
		};

		struct PerMeshGPU
		{
			math::Matrix   meshToModel;
			float          maxRadius;
			float          pad[3];
		};

		struct PerMaterialGPU
		{
			uint32_t   flags;
			float      roughness;
			float      metalness;
			float      pad[1];
		};

	public:
		struct PerMaterial
		{
			Material    material;
			SolidVector<InstanceData>  instances;
		};

		struct PerMesh
		{
			std::vector<PerMaterial>  perMaterial;
		};
		struct PerModel
		{
			PerModel(const std::shared_ptr<Model>& srcModel);
			bool operator==(const PerModel& rval) { return srcModel.get() == rval.srcModel.get(); }

			std::shared_ptr<Model>   srcModel;
			std::vector<PerMesh> perMesh;
		};

	private:

		renderer::PixelShader                         m_PS;
		renderer::VertexShader                        m_VS;

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