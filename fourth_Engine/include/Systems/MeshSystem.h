#pragma once
#include "include/Render/Renderer/ShadingGroups/HologramGroup.h"
#include "include/Render/Renderer/ShadingGroups/NormalVisGroup.h"
#include "include/Render/Renderer/ShadingGroups/TextureOnlyGroup.h"
#include "include/Render/Renderer/ShadingGroups/EmissionOnly.h"	
#include "include/Render/Renderer/ShadingGroups/DissolutionGroup.h"	
#include "include/Render/Renderer/ShadingGroups/OpaqueGroup.h"
#include "include/Render/Renderer/ShadingGroups/IncinerationGroup.h"
#include "include/Render/ShadowMap.h"

namespace fth
{
	struct TexturePack;
}

namespace fth
{
	struct Ray;
	struct QueryHitResult;


	class MeshSystem
	{
	public:

		enum class IntersectedType
		{
			NormalGroup,
			HologramGroup,
			TextureOnlyGroup,
			EmissionGroup,
			OpaqueGroup,
			DissolutionGroup,
			IncinerationGroup,
			NUM
		};

		struct HitResult
		{
			math::HitResult            hitResult;
			//std::unique_ptr<IDragger>* dragger;
			Handle<math::Matrix>       handle_transform;
			Handle<ModelInstance>      handle_instance;
			Handle<RenderableObject>   handle_object;
			IntersectedType            type;
		};



		static MeshSystem& Get()
		{
			static MeshSystem instance;
			return instance;
		}

	

		~MeshSystem();
		void Init();
		void Shutdown();
		void Update(float deltaTime);
		void Draw();
		void RenderDepth2D();
		void RenderDepthCubemaps(const std::vector<math::Vector3>& positions);
		void SubmitShadowMapUniformData(const math::Matrix& directionalVP, const math::Matrix& spotVP, const std::vector<ShadowMap::cbSMFrustumPlanesGPU>& frustums);
		void BindShadowmapUniform(uint16_t slot) const;


		//Adding a stream of instances is still a problem. If each instance needs to know beforehand its objectID and its global InstanceID, then we still have to
		//ask SolidVector next ID. However, ModelInstance struct has a vector as big as num of meshes. If each instance addition needs to output a stream of MeshInstances,
		//we still have to make N std::vector allocations in SolidVector...
		Handle<shading::HologramGroup::PerModel> addHologramModel(const std::shared_ptr<Model>& model);
		void addHologramMaterial(const shading::HologramGroup::Material* materials, Handle<shading::HologramGroup::PerModel> model, Handle<shading::HologramGroup::Material>* outHandles);
		void addHologramInstances(Handle<shading::HologramGroup::PerModel> model, const Handle<shading::HologramGroup::Material>* materials, shading::HologramGroup::InstanceData* data, uint32_t numInstances, uint32_t numMeshes);

		Handle<shading::NormalVisGroup::PerModel> addNormalVisModel(const std::shared_ptr<Model>& model);
		void addNormalVisMaterial(const shading::NormalVisGroup::Material* materials, Handle<shading::NormalVisGroup::PerModel> model, Handle<shading::NormalVisGroup::Material>* outHandles);
		void addNormalVisInstances(Handle<shading::NormalVisGroup::PerModel> model, const Handle<shading::NormalVisGroup::Material>* materials, shading::NormalVisGroup::InstanceData* data, uint32_t numInstances, uint32_t numMeshes);

		Handle<shading::EmissionOnly::PerModel> addEmissionOnlyModel(const std::shared_ptr<Model>& model);
		void addEmissionOnlyMaterial(const shading::EmissionOnly::Material* materials, Handle<shading::EmissionOnly::PerModel> model, Handle<shading::EmissionOnly::Material>* outHandles);
		void addEmissionOnlyInstances(Handle<shading::EmissionOnly::PerModel> model, const Handle<shading::EmissionOnly::Material>* materials, shading::EmissionOnly::InstanceData* data, uint32_t numInstances, uint32_t numMeshes);

		Handle<shading::TextureOnlyGroup::PerModel> addTextureOnlyModel(const std::shared_ptr<Model>& model);
		void addTextureOnlyMaterial(const shading::TextureOnlyGroup::Material* materials, Handle<shading::TextureOnlyGroup::PerModel> model, Handle<shading::TextureOnlyGroup::Material>* outHandles);
		void addTextureOnlyInstances(Handle<shading::TextureOnlyGroup::PerModel> model, const Handle<shading::TextureOnlyGroup::Material>* materials, shading::TextureOnlyGroup::InstanceData* data, uint32_t numInstances, uint32_t numMeshes);

		Handle<shading::OpaqueGroup::PerModel> addOpaqueModel(const std::shared_ptr<Model>& model);
		void addOpaqueMaterial(const shading::OpaqueGroup::Material* materials, Handle<shading::OpaqueGroup::PerModel> model, Handle<shading::OpaqueGroup::Material>* outHandles);
		void addOpaqueInstances(Handle<shading::OpaqueGroup::PerModel> model, const Handle<shading::OpaqueGroup::Material>* materials, shading::OpaqueGroup::InstanceData* data, uint32_t numInstances, uint32_t numMeshes);
		bool queryInstanceMaterial(Handle<ModelInstance> targetInstance, shading::OpaqueGroup::Material* outMaterials) const;

		Handle<shading::DissolutionGroup::PerModel> addDissolutionModel(const std::shared_ptr<Model>& model);
		void addDissolutionMaterial(const shading::DissolutionGroup::Material* materials, Handle<shading::DissolutionGroup::PerModel> model, Handle<shading::DissolutionGroup::Material>* outHandles);
		void addDissolutionInstances(Handle<shading::DissolutionGroup::PerModel> model, const Handle<shading::DissolutionGroup::Material>* materials, shading::DissolutionGroup::InstanceData* data, uint32_t numInstances, uint32_t numMeshes);

		const shading::DissolutionGroup::InstanceData& getDissolutionInstanceData(Handle<ModelInstance> targetInstance) const;
		shading::DissolutionGroup::InstanceData& getDissolutionInstanceData(Handle<ModelInstance> targetInstance);
		//void UpdateDissolutionInstanceAnimation(Handle<ModelInstance> id, float currentTime);
		//float GetDissolutionInstanceCurrentTime(Handle<ModelInstance> id);
		////const shading::DissolutionGroup::InstanceData& GetDissolutionInstanceData(Handle<ModelInstance> id);
		//const std::shared_ptr<Model>& GetDissolutionInstanceModel(Handle<ModelInstance> id);

		Handle<shading::IncinerationGroup::PerModel> addIncinerationModel(const std::shared_ptr<Model>& model);
		void addIncinerationMaterial(const shading::IncinerationGroup::Material* materials, Handle<shading::IncinerationGroup::PerModel> model, Handle<shading::IncinerationGroup::Material>* outHandles);
		void addIncinerationInstances(Handle<shading::IncinerationGroup::PerModel> model, const Handle<shading::IncinerationGroup::Material>* materials, shading::IncinerationGroup::InstanceData* data, uint32_t numInstances, uint32_t numMeshes);

		const shading::IncinerationGroup::InstanceData& getIncinerationInstanceData(Handle<ModelInstance> target) const;
	    shading::IncinerationGroup::InstanceData getIncinerationInstanceData(Handle<ModelInstance> target);

		void deleteInstance(Handle<ModelInstance>& handle);
		const std::shared_ptr<Model>& getModelByHandle(Handle<ModelInstance> instanceHandle) const;

		void ToggleNormalDebugging();
		bool FindIntersection(const Ray& ray, MeshSystem::HitResult& outQuery);
	private:
		static constexpr uint32_t shadingMask{ 0xFF };
		MeshSystem();
		Handle<RenderableObject> incrementObjectID() { return m_nextID++; };

		//gets next ID without increasing it
		Handle<RenderableObject> getNextObjectID() const { return m_nextID; }

		enum ShadingGroupIDs
		{
			NORMAL = 0,
			HOLOGRAM,
			TEXONLY,
			OPAQUE_LIT,
			EMISSION,
			DISSOLUTION,
			INCINERATION
		};
	protected:
		shading::HologramGroup         m_hologramVis;
		shading::NormalVisGroup        m_normalVis;
		shading::TextureOnlyGroup      m_textureOnlyVis;
		shading::OpaqueGroup           m_opaque;
		shading::EmissionOnly          m_emissionOnlyVis;
		shading::DissolutionGroup      m_dissolutionOpaque;
		shading::IncinerationGroup     m_incineration;

		Handle<RenderableObject>       m_nextID = 0;
		std::vector<math::Matrix>      m_cubeShadowVP_cache;
		renderer::UniformBuffer        m_shadowmapUniform;
	
	
		SolidVector<ModelInstance>     m_modelInstances;
	};
	
}