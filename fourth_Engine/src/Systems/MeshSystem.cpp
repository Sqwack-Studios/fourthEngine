#include "pch.h"

#pragma once
#include "include/Systems/MeshSystem.h"
#include "include/Math/Ray.h"
#include "include/Render/Renderer/D3DRenderer.h"
#include "include/Render/Dragger.h"
#include "include/Managers/TextureManager.h"
#include "include/Systems/TransformSystem.h"
#include "include/Systems/LightSystem.h"
#include "../Shaders/RegisterSlots.hlsli"
#include "include/Specifications.h"




namespace fth
{

	MeshSystem::MeshSystem() {};
	MeshSystem::~MeshSystem() {}
	void MeshSystem::Init()
	{
		//Load depth-pass shaders, set them into those shading groups that need them
		
		//Load shaders 
		m_hologramVis.Init(7, 1, 16);
		m_normalVis.Init(7, 1, 16);
		m_textureOnlyVis.Init(5, 8, 11);
		m_opaque.Init(5, 8, 11);
		m_dissolutionOpaque.Init(5, 8, 11);
		m_emissionOnlyVis.Init(7, 1, 16);
		m_incineration.Init(5, 8, 11);

		m_opaque.SetDepthPass_2D(renderer::D3DRenderer::Get().GetDepthPass_2D_VertexShader());
		m_opaque.SetDepthPass_Cube(renderer::D3DRenderer::Get().GetDepthPass_Cube_VertexShader(), renderer::D3DRenderer::Get().GetDepthPass_Cube_GeometryShader());


		m_cubeShadowVP_cache.resize(Config::MAX_POINT_LIGHTS);
		m_shadowmapUniform.CreateGPUBuffer(sizeof(ShadowMap::cbShadowMapsGPU), 1, nullptr);

		//Initialize buffer to avoid driver warnings about uninitialized data
		ShadowMap::cbShadowMapsGPU dummy;
		void* map = m_shadowmapUniform.Map();
		memcpy(map, &dummy, sizeof(ShadowMap::cbShadowMapsGPU));
		m_shadowmapUniform.Unmap();
	}

	void MeshSystem::Shutdown()
	{

	}

	void MeshSystem::Update(float deltaTime)
	{
		m_dissolutionOpaque.update(deltaTime);
		m_incineration.update(deltaTime);

		m_hologramVis.UpdateInstanceBuffer();
		m_normalVis.UpdateInstanceBuffer();
		m_opaque.UpdateInstanceBuffer();
		m_textureOnlyVis.UpdateInstanceBuffer();
		m_emissionOnlyVis.UpdateInstanceBuffer();
		m_dissolutionOpaque.UpdateInstanceBuffer();
		m_incineration.UpdateInstanceBuffer();
	}
	void MeshSystem::Draw()
	{
		if (renderer::D3DRenderer::Get().WireframeEnabled())
		{
			renderer::D3DRenderer::Get().BindRasterizerState(renderer::RSTYPES::RSTYPE_NOCULL_WIREFRAME);
		}
		else
		{
			renderer::D3DRenderer::Get().BindRasterizerState(renderer::RSTYPES::DEFAULT);
		}
		renderer::D3DRenderer::Get().BindDepthStencilState(renderer::DSSTYPES::DSS_DEPTH_RW_GREATER_STENCIL_RW_OPALWAYS, renderer::StencilValues::NO_LIGHTNING);
		
		//Keep grouping if they tesselate or not, to reduce  binding shader calls when rendering debug normals? Its debugging, doest it really mater?xd

		//Render first tesselated 
		//Tesselated shading groups render their debug normals right after their normal rendering to avoid shader rebinding, but they have
		//to call renderer to bind them the PixelShader each time



		m_hologramVis.Render();
		//m_hologramVis.RenderDebugNormals();

		renderer::D3DRenderer::Get().ClearTesselationStage();
		renderer::D3DRenderer::Get().ClearGeometryStage();

		////Render non tesselated
		////Non-tesselated normals are rendered once normal renderin occurs for everyone, so they avoid PS binding

		m_normalVis.Render();
		m_textureOnlyVis.Render();
		m_emissionOnlyVis.Render();



		renderer::D3DRenderer::Get().BindDepthStencilState(renderer::DSSTYPES::DSS_DEPTH_RW_GREATER_STENCIL_RW_OPALWAYS, renderer::StencilValues::PBR_SHADOWS);

		m_opaque.Render();
		////renderer::D3DRenderer::Get().BindBlendState(renderer::BSTYPES::DEFAULT_A2C);
		m_dissolutionOpaque.Render();
		m_incineration.Render();


		//renderer::D3DRenderer::Get().BindPSNormalDebug();
		////renderer::D3DRenderer::Get().BindBlendState(renderer::BSTYPES::DEFAULT);

		//m_normalVis.RenderDebugNormals();
		//m_textureOnlyVis.RenderDebugNormals();
		//m_emissionOnlyVis.RenderDebugNormals();
		//m_opaque.RenderDebugNormals();


		renderer::D3DRenderer::Get().ClearGeometryStage();


		//To tryhard more, because we are clearing geometry stage many times, we could first update instances and buffers, and render normal debugging
		//right before clearing tesselation and geometry stages. So normalVis shader would have its normals drawn first, and then render normally.¿?
	}

	

	void MeshSystem::RenderDepth2D()
	{
		renderer::D3DRenderer::Get().BindDepthStencilState(renderer::DSSTYPES::DSS_DEPTH_RW_GREATER_STENCIL_DISABLED);
		renderer::D3DRenderer::Get().BindRasterizerState(renderer::RSTYPES::RSTYPE_DEFAULT_DEPTH_BIAS);

		//Needs pixel shader 
		m_dissolutionOpaque.RenderDepth2DOnly();
		m_incineration.RenderDepth2DOnly();

		renderer::PixelShader::ClearFromPipeline();
		m_opaque.RenderDepth2DOnly();
	}

	void MeshSystem::RenderDepthCubemaps(const std::vector<math::Vector3>&  positions)
	{
		renderer::D3DRenderer::Get().BindDepthStencilState(renderer::DSSTYPES::DSS_DEPTH_RW_GREATER_STENCIL_DISABLED);
		renderer::D3DRenderer::Get().BindRasterizerState(renderer::RSTYPES::RSTYPE_DEFAULT_DEPTH_BIAS);

		const ShadowMap& pointShadowmap{ LightSystem::Get().GetPointShadowmap() };

		uint16_t numCubemaps{};
		{
			math::Matrix* data = m_cubeShadowVP_cache.data();
			for (const math::Vector3& pos : positions)
			{
				const std::vector<math::Matrix>& vpStream = pointShadowmap.ComputeFrustumPerspectiveStream(pos);
				memcpy(data, vpStream.data(), sizeof(math::Matrix) * 6);
				data += 6;
				++numCubemaps;
			}
		}

		const math::Matrix* vpData = m_cubeShadowVP_cache.data();

		m_dissolutionOpaque.RenderDepthCubemapsOnly(vpData, numCubemaps);
		m_incineration.RenderDepthCubemapsOnly(vpData, numCubemaps);

		renderer::PixelShader::ClearFromPipeline();

		m_opaque.RenderDepthCubemapsOnly(vpData, numCubemaps);
	}

	void MeshSystem::ToggleNormalDebugging()
	{
		//m_hologramVis.ToggleNormalDebugging();
		//m_normalVis.ToggleNormalDebugging();
		//m_textureOnlyVis.ToggleNormalDebugging();
		//m_emissionOnlyVis.ToggleNormalDebugging();
		m_opaque.ToggleNormalDebugging();
	}

	Handle<shading::HologramGroup::PerModel> MeshSystem::addHologramModel(const std::shared_ptr<Model>& model)
	{
		return m_hologramVis.addModel(model);
	}

	void MeshSystem::addHologramMaterial(const shading::HologramGroup::Material* materials, Handle<shading::HologramGroup::PerModel> model, Handle<shading::HologramGroup::Material>* outHandles)
	{
		m_hologramVis.addMaterial(materials, model, outHandles);
	}

	void MeshSystem::addHologramInstances(Handle<shading::HologramGroup::PerModel> model, const Handle<shading::HologramGroup::Material>* materials, shading::HologramGroup::InstanceData* data, uint32_t numInstances, uint32_t numMeshes)
	{
		ModelInstance temp;
		temp.perMeshInstance.reserve(numMeshes);
		for (uint32_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
		{
			temp.perMeshInstance.emplace_back(ShadingGroupIDs::HOLOGRAM << 24);
		}

		for (uint32_t instance = 0; instance < numInstances; ++instance)
		{
			shading::HologramGroup::InstanceData& ref{ data[instance] };
			ref.handle_modelInstance = m_modelInstances.queryNextHandle();
			ref.handle_object = getNextObjectID();

			m_hologramVis.addInstances(model, materials, &ref, 1, temp.perMeshInstance.data());
			m_modelInstances.insert(temp);
			incrementObjectID();
		}
	}


	Handle<shading::NormalVisGroup::PerModel> MeshSystem::addNormalVisModel(const std::shared_ptr<Model>& model)
	{
		return m_normalVis.addModel(model);
	}

	void MeshSystem::addNormalVisMaterial(const shading::NormalVisGroup::Material* materials, Handle<shading::NormalVisGroup::PerModel> model, Handle<shading::NormalVisGroup::Material>* outHandles)
	{
		m_normalVis.addMaterial(materials, model, outHandles);
	}

	void MeshSystem::addNormalVisInstances(Handle<shading::NormalVisGroup::PerModel> model, const Handle<shading::NormalVisGroup::Material>* materials, shading::NormalVisGroup::InstanceData* data, uint32_t numInstances, uint32_t numMeshes)
	{
		ModelInstance temp;
		temp.perMeshInstance.reserve(numMeshes);
		for (uint32_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
		{
			temp.perMeshInstance.emplace_back(ShadingGroupIDs::NORMAL << 24);
		}

		for (uint32_t instance = 0; instance < numInstances; ++instance)
		{
			shading::NormalVisGroup::InstanceData& ref{ data[instance] };
			ref.handle_modelInstance = m_modelInstances.queryNextHandle();
			ref.handle_object = getNextObjectID();

			m_normalVis.addInstances(model, materials, &ref, 1, temp.perMeshInstance.data());
			m_modelInstances.insert(temp);
			incrementObjectID();
		}
	}


	Handle<shading::EmissionOnly::PerModel> MeshSystem::addEmissionOnlyModel(const std::shared_ptr<Model>& model)
	{
		return m_emissionOnlyVis.addModel(model);
	}

	void MeshSystem::addEmissionOnlyMaterial(const shading::EmissionOnly::Material* materials, Handle<shading::EmissionOnly::PerModel> model, Handle<shading::EmissionOnly::Material>* outHandles)
	{
		m_emissionOnlyVis.addMaterial(materials, model, outHandles);
	}

	void MeshSystem::addEmissionOnlyInstances(Handle<shading::EmissionOnly::PerModel> model, const Handle<shading::EmissionOnly::Material>* materials, shading::EmissionOnly::InstanceData* data, uint32_t numInstances, uint32_t numMeshes)
	{
		ModelInstance temp;
		temp.perMeshInstance.reserve(numMeshes);
		for (uint32_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
		{
			temp.perMeshInstance.emplace_back(ShadingGroupIDs::EMISSION << 24);
		}

		for (uint32_t instance = 0; instance < numInstances; ++instance)
		{
			shading::EmissionOnly::InstanceData& ref{ data[instance] };
			ref.handle_modelInstance = m_modelInstances.queryNextHandle();
			ref.handle_object = getNextObjectID();

			m_emissionOnlyVis.addInstances(model, materials, &ref, 1, temp.perMeshInstance.data());
			m_modelInstances.insert(temp);
			incrementObjectID();
		}
	}


	Handle<shading::TextureOnlyGroup::PerModel> MeshSystem::addTextureOnlyModel(const std::shared_ptr<Model>& model)
	{
		return m_textureOnlyVis.addModel(model);
	}

	void MeshSystem::addTextureOnlyMaterial(const shading::TextureOnlyGroup::Material* materials, Handle<shading::TextureOnlyGroup::PerModel> model, Handle<shading::TextureOnlyGroup::Material>* outHandles)
	{
		m_textureOnlyVis.addMaterial(materials, model, outHandles);
	}

	void MeshSystem::addTextureOnlyInstances(Handle<shading::TextureOnlyGroup::PerModel> model, const Handle<shading::TextureOnlyGroup::Material>* materials, shading::TextureOnlyGroup::InstanceData* data, uint32_t numInstances, uint32_t numMeshes)
	{
		ModelInstance temp;
		temp.perMeshInstance.reserve(numMeshes);
		for (uint32_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
		{
			temp.perMeshInstance.emplace_back(ShadingGroupIDs::OPAQUE_LIT << 24);
		}
		for (uint32_t instance = 0; instance < numInstances; ++instance)
		{
			shading::TextureOnlyGroup::InstanceData& ref{ data[instance] };
			ref.handle_modelInstance = m_modelInstances.queryNextHandle();
			ref.handle_object = getNextObjectID();

			m_textureOnlyVis.addInstances(model, materials, &ref, 1, temp.perMeshInstance.data());
			m_modelInstances.insert(temp);
			incrementObjectID();
		}
	}




	Handle<shading::OpaqueGroup::PerModel> MeshSystem::addOpaqueModel(const std::shared_ptr<Model>& model)
	{
		return m_opaque.addModel(model);
	}

	void MeshSystem::addOpaqueMaterial(
		const shading::OpaqueGroup::Material* materials, 
		Handle<shading::OpaqueGroup::PerModel> model, 
		Handle<shading::OpaqueGroup::Material>* outHandles)
	{
		m_opaque.addMaterial(materials, model, outHandles);
	}

	void MeshSystem::addOpaqueInstances(
		Handle<shading::OpaqueGroup::PerModel> model, 
		const Handle<shading::OpaqueGroup::Material>* materials, 
		shading::OpaqueGroup::InstanceData* data, 
		uint32_t numInstances,
		uint32_t numMeshes)
	{
		//query which should be next instance ID
		//query which should be next object ID

		//Adding a stream of instances is still a problem. If each instance needs to know beforehand its objectID and its global InstanceID, then we still have to
		//ask SolidVector next ID. However, ModelInstance struct has a vector as big as num of meshes. If each instance addition needs to output a stream of MeshInstances,
		//we still have to make N std::vector allocations in SolidVector...
		ModelInstance temp;
		temp.perMeshInstance.reserve(numMeshes);
		for (uint32_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
		{
			temp.perMeshInstance.emplace_back(ShadingGroupIDs::OPAQUE_LIT << 24);
		}
		for (uint32_t instance = 0; instance < numInstances; ++instance)
		{
			shading::OpaqueGroup::InstanceData& ref{ data[instance] };
			ref.handle_modelInstance = m_modelInstances.queryNextHandle();
			ref.handle_object = getNextObjectID();

			m_opaque.addInstances(model, materials, &ref, 1, temp.perMeshInstance.data());
			m_modelInstances.insert(temp);
			incrementObjectID();

		}
	}

	bool MeshSystem::queryInstanceMaterial(Handle<ModelInstance> instance, shading::OpaqueGroup::Material* data) const
	{
		//check that provided Handle actually is an opaque instance
		const ModelInstance& modelInstance{ m_modelInstances[instance].perMeshInstance };
		uint32_t numMeshes{ static_cast<uint32_t>(modelInstance.perMeshInstance.size()) };
		ShadingGroupIDs shadingID{ static_cast<ShadingGroupIDs>((modelInstance.perMeshInstance[0].composedID >> 24) & shadingMask) };

		if (shadingID != ShadingGroupIDs::OPAQUE_LIT)
			return false;

		return m_opaque.queryInstanceMaterial(&modelInstance.perMeshInstance.front().composedID, numMeshes, data);
	}

	Handle<shading::DissolutionGroup::PerModel> MeshSystem::addDissolutionModel(const std::shared_ptr<Model>& model)
	{
		return m_dissolutionOpaque.addModel(model);
	}

	void MeshSystem::addDissolutionMaterial(const shading::DissolutionGroup::Material* materials, Handle<shading::DissolutionGroup::PerModel> model, Handle<shading::DissolutionGroup::Material>* outHandles)
	{
		m_dissolutionOpaque.addMaterial(materials, model, outHandles);
	}

	void MeshSystem::addDissolutionInstances(Handle<shading::DissolutionGroup::PerModel> model, const Handle<shading::DissolutionGroup::Material>* materials, shading::DissolutionGroup::InstanceData* data, uint32_t numInstances, uint32_t numMeshes)
	{
//query which should be next instance ID
//query which should be next object ID

		ModelInstance temp;
		temp.perMeshInstance.reserve(numMeshes);
		for (uint32_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
		{
			temp.perMeshInstance.emplace_back(ShadingGroupIDs::DISSOLUTION << 24);
		}
		for (uint32_t instance = 0; instance < numInstances; ++instance)
		{
			shading::DissolutionGroup::InstanceData& ref{ data[instance] };
			ref.handle_modelInstance = m_modelInstances.queryNextHandle();
			ref.handle_object = getNextObjectID();

			m_dissolutionOpaque.addInstances(model, materials, &ref, 1, temp.perMeshInstance.data());
			m_modelInstances.insert(temp);
			incrementObjectID();

		}
	}


	const shading::DissolutionGroup::InstanceData& MeshSystem::getDissolutionInstanceData(Handle<ModelInstance> targetInstance) const
	{
		return m_dissolutionOpaque.getInstanceData(m_modelInstances[targetInstance].perMeshInstance.front());
	}

	shading::DissolutionGroup::InstanceData& MeshSystem::getDissolutionInstanceData(Handle<ModelInstance> targetInstance)
	{
		return const_cast<shading::DissolutionGroup::InstanceData&>(
			static_cast<const MeshSystem&>(*this).getDissolutionInstanceData(targetInstance)
			);
	}

	const shading::IncinerationGroup::InstanceData& MeshSystem::getIncinerationInstanceData(Handle<ModelInstance> target) const
	{
		return m_incineration.getInstanceData(m_modelInstances[target].perMeshInstance.front());
	}

	shading::IncinerationGroup::InstanceData MeshSystem::getIncinerationInstanceData(Handle<ModelInstance> target)
	{
		return const_cast<shading::IncinerationGroup::InstanceData&>(
			static_cast<const MeshSystem&>(*this).getIncinerationInstanceData(target)
			);
	}

	Handle<shading::IncinerationGroup::PerModel> MeshSystem::addIncinerationModel(const std::shared_ptr<Model>& model)
	{
		return m_incineration.addModel(model);
	}

	void MeshSystem::addIncinerationMaterial(const shading::IncinerationGroup::Material* materials, Handle<shading::IncinerationGroup::PerModel> model, Handle<shading::IncinerationGroup::Material>* outHandles)
	{
		m_incineration.addMaterial(materials, model, outHandles);
	}

	void MeshSystem::addIncinerationInstances(Handle<shading::IncinerationGroup::PerModel> model, const Handle<shading::IncinerationGroup::Material>* materials, shading::IncinerationGroup::InstanceData* data, uint32_t numInstances, uint32_t numMeshes)
	{
		ModelInstance temp;
		temp.perMeshInstance.reserve(numMeshes);
		for (uint32_t meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
		{
			temp.perMeshInstance.emplace_back(ShadingGroupIDs::INCINERATION << 24);
		}
		for (uint32_t instance = 0; instance < numInstances; ++instance)
		{
			shading::IncinerationGroup::InstanceData& ref{ data[instance] };
			ref.handle_modelInstance = m_modelInstances.queryNextHandle();
			ref.handle_object = getNextObjectID();

			m_incineration.addInstances(model, materials, &ref, 1, temp.perMeshInstance.data());
			m_modelInstances.insert(temp);
			incrementObjectID();
		}
	}


	void MeshSystem::deleteInstance(Handle<ModelInstance>& handle)
	{
		const ModelInstance& modelInstance{ m_modelInstances[handle].perMeshInstance };

		uint32_t numMeshes{ static_cast<uint32_t>(modelInstance.perMeshInstance.size()) };
		ShadingGroupIDs shadingID{ static_cast<ShadingGroupIDs>( (modelInstance.perMeshInstance[0].composedID >> 24) & shadingMask )};
		
		bool wasDeleted{ };

		switch (shadingID)
		{
		case fth::MeshSystem::NORMAL:
			break;
		case fth::MeshSystem::HOLOGRAM:
			break;
		case fth::MeshSystem::TEXONLY:
			break;
		case fth::MeshSystem::OPAQUE_LIT:
		{
			wasDeleted = m_opaque.deleteInstance(&modelInstance.perMeshInstance.front().composedID, numMeshes);
			break;
		}
		case fth::MeshSystem::EMISSION:
			break;
		case fth::MeshSystem::DISSOLUTION:
		{
			wasDeleted = m_dissolutionOpaque.deleteInstance(&modelInstance.perMeshInstance.front().composedID, numMeshes);
			break;
		}
		case fth::MeshSystem::INCINERATION:
		{
			wasDeleted = m_incineration.deleteInstance(&modelInstance.perMeshInstance.front().composedID, numMeshes);
		}
		default:
			break;
		}

		if(!wasDeleted)
		{
			BREAK_AND_LOG_ENGINE("wasDeleted != true", "Deletion failed. ShadingID {0}, instance Handle {1}", shadingID, handle.id);
		}

		m_modelInstances.erase(handle);
		handle.invalidate();
	}

	const std::shared_ptr<Model>& MeshSystem::getModelByHandle(Handle<ModelInstance> handle) const
	{
		const ModelInstance& modelInstance{ m_modelInstances[handle].perMeshInstance };

		uint32_t numMeshes{ static_cast<uint32_t>(modelInstance.perMeshInstance.size()) };
		ShadingGroupIDs shadingID{ static_cast<ShadingGroupIDs>((modelInstance.perMeshInstance[0].composedID >> 24) & shadingMask) };


		switch (shadingID)
		{
		case fth::MeshSystem::NORMAL:
			break;
		case fth::MeshSystem::HOLOGRAM:
			break;
		case fth::MeshSystem::TEXONLY:
			break;
		case fth::MeshSystem::OPAQUE_LIT:
		{
			return m_opaque.getModelByHandle(modelInstance.perMeshInstance.front().composedID);
			break;
		}
		case fth::MeshSystem::EMISSION:
		{
			break;
		}
		case fth::MeshSystem::DISSOLUTION:
		{
			return m_dissolutionOpaque.getModelByHandle(modelInstance.perMeshInstance.front().composedID);
			break;
		}
		case fth::MeshSystem::INCINERATION:
		{
			break;
		}
		default:
			return nullptr;
			break;
		}

	}
	
	bool MeshSystem::FindIntersection(const Ray& ray, MeshSystem::HitResult& outQuery)
	{
		//Find intersection with every AABB
		//We have to compute the intersection foreach mesh for every AABB intersected
		//
		shading::InstanceFinder closestModel;
		closestModel.invalidate();
		bool foundMeshIntersect{};

		IntersectedType type = IntersectedType::NUM;
		//Find AABB foreach shader for each instance model

		if (m_normalVis.FindIntersection(ray, outQuery.hitResult, closestModel))
		{
			foundMeshIntersect = true;
			type = IntersectedType::NormalGroup;
		}
		if (m_hologramVis.FindIntersection(ray, outQuery.hitResult, closestModel))
		{
			foundMeshIntersect = true;
			type = IntersectedType::HologramGroup;
		}
		if (m_textureOnlyVis.FindIntersection(ray, outQuery.hitResult, closestModel))
		{
			foundMeshIntersect = true;
			type = IntersectedType::TextureOnlyGroup;
		}
		if (m_emissionOnlyVis.FindIntersection(ray, outQuery.hitResult, closestModel))
		{
			foundMeshIntersect = true;
			type = IntersectedType::EmissionGroup;
		}
		if (m_opaque.FindIntersection(ray, outQuery.hitResult, closestModel))
		{
			foundMeshIntersect = true;
			type = IntersectedType::OpaqueGroup;
		}
		if (m_dissolutionOpaque.FindIntersection(ray, outQuery.hitResult, closestModel))
		{
			foundMeshIntersect = true;
			type = IntersectedType::DissolutionGroup;
		}
		if (m_incineration.FindIntersection(ray, outQuery.hitResult, closestModel))
		{
			foundMeshIntersect = true;
			type = IntersectedType::IncinerationGroup;
		}
		outQuery.type = type;
		outQuery.handle_transform = closestModel.handle_transform;
		outQuery.handle_object = closestModel.handle_object;
		outQuery.handle_instance = closestModel.handle_instance;

		return foundMeshIntersect;
	}

	void MeshSystem::SubmitShadowMapUniformData(const math::Matrix& directionalVP, const math::Matrix& spotVP, const std::vector<ShadowMap::cbSMFrustumPlanesGPU>& frustums)
	{
		//Map uniform
		//Copy directional and spot matrices
		//Copy point matrices

		ShadowMap::cbShadowMapsGPU* map = static_cast<ShadowMap::cbShadowMapsGPU*>(m_shadowmapUniform.Map());

		(*map).directionalVP = directionalVP;
		(*map).spotVP = spotVP;
		memcpy(&(*map).pointVP, m_cubeShadowVP_cache.data(), sizeof(math::Matrix) * m_cubeShadowVP_cache.size());
		memcpy(&(*map).frustumPlanes, frustums.data(), sizeof(ShadowMap::cbSMFrustumPlanesGPU) * frustums.size());

		m_shadowmapUniform.Unmap();
	}

	void MeshSystem::BindShadowmapUniform(uint16_t slot) const
	{
		m_shadowmapUniform.BindPS(slot);
	}

	
}