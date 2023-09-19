#include "pch.h"
#pragma once
#include "include/Render/Renderer/ShadingGroups/OpaqueGroup.h"
#include "include/Render/Model.h"
#include "include/Math/Ray.h"
#include "include/Render/Renderer/D3D11Headers.h"
#include "include/Managers/ModelManager.h"
#include "include/Managers/TextureManager.h"
#include "Shaders/RegisterSlots.hlsli"
#include "include/Managers/CameraManager.h"
#include "include/Render/ShadowMap.h"



namespace fth
{
	extern ID3D11DeviceContext4* s_devcon;
}
namespace fth::shading
{
	OpaqueGroup::PerModel::PerModel(const std::shared_ptr<Model>& srcModel):
		srcModel(srcModel)
	{
		perMesh.resize(srcModel->GetNumMeshes());
	}

	OpaqueGroup::Material::Material(
		const std::shared_ptr<Texture>& albedo,
		const std::shared_ptr<Texture>& normal,
		const std::shared_ptr<Texture>& roughness,
		const std::shared_ptr<Texture>& metalness,
		uint16_t inFlags,
		float f_roughness,
		float f_metalness) :

		albedo(albedo),
		normal(normal),
		roughness(roughness),
		metalness(metalness),
		flags(inFlags),
		f_roughness(f_roughness),
		f_metalness(f_metalness)
	{
		if (normal)
			flags |= shading::OpaqueGroup::MaterialFlags::HAS_NORMALS;
		if (roughness)
			flags |= shading::OpaqueGroup::MaterialFlags::HAS_ROUGHNESS;
		if (metalness)
			flags |= shading::OpaqueGroup::MaterialFlags::HAS_METALNESS;
	}

	void OpaqueGroup::Material::Bind() const
	{
		albedo->BindPS(TX_ALBEDO_SLOT);

		if (flags & HAS_NORMALS)
			normal->BindPS(TX_NORMAL_SLOT);

		if(flags & HAS_ROUGHNESS)
			roughness->BindPS(TX_ROUGH_SLOT);

		if(flags & HAS_METALNESS)
			metalness->BindPS(TX_METAL_SLOT);
	}


	void OpaqueGroup::Init(uint8_t inModelBits, uint8_t inMaterialBits, uint8_t inInstanceBits)
	{
		IShadingGroup::Init(inModelBits, inMaterialBits, inInstanceBits);

		const D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,  0, 0                        , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",   0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,  0, 1 * sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT",  0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,  0, 2 * sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BITANGENT",0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,  0, 3 * sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"UV",       0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,     0, 4 * sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0},

			{"MW",       0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0                                       ,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MW",       1, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1 * sizeof(math::Vector4)               ,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MW",       2, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 2 * sizeof(math::Vector4)               ,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MW",       3, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 3 * sizeof(math::Vector4)               ,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"ID",       0, DXGI_FORMAT::DXGI_FORMAT_R32_UINT          , 1,     sizeof(math::Matrix)                ,  D3D11_INPUT_PER_INSTANCE_DATA, 1}

		};



		m_VS.LoadShader(L"OpaqueGroup_VS.cso", ied, std::size(ied));
		m_PS.LoadShader(L"OpaqueGroup_PS.cso");
		m_normalDebugGS.LoadShader(L"OpaqueGroup_NormalDebug_GS.cso");
		m_perMeshUniform.CreateGPUBuffer(sizeof(PerMeshDataGPU), 1, nullptr);
		m_perMaterialUniform.CreateGPUBuffer(sizeof(PerMaterialGPU), 1, nullptr);
		m_depthPassCube_Uniform.CreateGPUBuffer(sizeof(ShadowMap::CubemapDepthPassBuffer), 1, nullptr);

		m_numInstances = 0;
	}

	void OpaqueGroup::SetDepthPass_2D(const renderer::VertexShader& shader)
	{
		m_depthPass2D_VS = shader;
	}

	void OpaqueGroup::SetDepthPass_Cube(
		const renderer::VertexShader& vs, 
		const renderer::GeometryShader& gs)
	{
		m_depthPassCube_VS = vs;
		m_depthPassCube_GS = gs;
	}

	void OpaqueGroup::UpdateInstanceBuffer()
	{
		if (m_perModel.empty() || !m_numInstances)
			return;

		m_instanceBuffer.CreateGPUBuffer(sizeof(InstanceDataGPU), m_numInstances, nullptr);
		InstanceDataGPU* dst = static_cast<InstanceDataGPU*>(m_instanceBuffer.Map());

		uint32_t copiedNum{};

#ifdef CAMERA_CENTER_WS
		math::Matrix cameraPos{};
		cameraPos.Translation(CameraManager::GetActiveCamera().GetPosition());
#endif
		for (const PerModel& perModel : m_perModel)
		{
			for (const PerMesh& perMesh : perModel.perMesh)
			{
				for (const PerMaterial& perMaterial : perMesh.perMaterial)
				{
					uint32_t instances{ static_cast<uint32_t>(perMaterial.instances.size()) };
					for (uint32_t instanceIdx = 0; instanceIdx < instances; ++instanceIdx)
					{
						const InstanceData& instance{ perMaterial.instances.at(instanceIdx) };
						
						const math::Matrix& transform = instance.handle_modelToWorld.isValid() ? TransformSystem::Get().QueryTransformMatrix(instance.handle_modelToWorld)
							                                         : math::Matrix::Identity;
#ifdef CAMERA_CENTER_WS
						const InstanceDataGPU& src{ transform - cameraPos, instance.handle_object.id };
#else
						const InstanceDataGPU& src{ transform, instance.handle_object.id};
#endif


						dst[copiedNum++] = src;
					}
				}
			}
		}

		m_instanceBuffer.Unmap();
	}

	void OpaqueGroup::Render()
	{
		if (m_numInstances == 0)
			return;

		m_PS.Bind();
		m_VS.Bind();
		RenderInternal();
	}

	void OpaqueGroup::RenderDebugNormals()
	{
		if (!ShouldDebugNormals())
			return;

		m_VS.Bind();
		m_normalDebugGS.Bind();
		m_instanceBuffer.Bind(1);
		RenderInternal();
	}

	void OpaqueGroup::RenderDepth2DOnly()
	{
		if (m_numInstances == 0)
			return;
		m_depthPass2D_VS.Bind();
		m_instanceBuffer.Bind(1);


		uint32_t renderedInstances{};

		for (const PerModel& perModel : m_perModel)
		{
			uint32_t meshCount{ static_cast<uint32_t>(perModel.perMesh.size()) };

			perModel.srcModel->BindVertexBuffer();
			bool hasIndexBuffer{ perModel.srcModel->BindIndexBuffer() };


			for (uint32_t meshIndex = 0; meshIndex < meshCount; ++meshIndex)
			{
				const Mesh& mesh = perModel.srcModel->GetMeshes().at(meshIndex);
				const Mesh::MeshRange& range = mesh.m_meshRange;

				PerMeshDataGPU* meshDataMap = static_cast<PerMeshDataGPU*>(m_perMeshUniform.Map());
				memcpy(meshDataMap, &mesh.m_meshToModel, sizeof(mesh.m_meshToModel));
				m_perMeshUniform.Unmap();
				m_perMeshUniform.BindVS(PER_DRAW_SLOT);

				uint32_t instancesToDraw{};
				for (const PerMaterial& perMaterial : perModel.perMesh[meshIndex].perMaterial)
				{
					instancesToDraw += static_cast<uint32_t>(perMaterial.instances.size());
				}

				if (!instancesToDraw)
					continue;

				if (hasIndexBuffer)
				{
					DX_CALL(s_devcon->DrawIndexedInstanced(range.indexNum, instancesToDraw, range.indexOffset, range.vertexOffset, renderedInstances));
				}
				else
				{
					DX_CALL(s_devcon->DrawInstanced(range.vertexNum, instancesToDraw, range.vertexOffset, renderedInstances));
				}
				renderedInstances += instancesToDraw;
			}

		}
	}

	void OpaqueGroup::RenderDepthCubemapsOnly(const math::Matrix* vp, uint16_t numCubemaps)
	{
		if (m_numInstances == 0)
			return;

		uint32_t renderedInstances{};

		m_depthPassCube_VS.Bind();
		m_depthPassCube_GS.Bind();
		m_instanceBuffer.Bind(1);


		for (const PerModel& perModel : m_perModel)
		{
			uint32_t meshCount{ static_cast<uint32_t>(perModel.perMesh.size()) };

			perModel.srcModel->BindVertexBuffer();
			bool hasIndexBuffer{ perModel.srcModel->BindIndexBuffer() };

			for(uint32_t meshIndex = 0; meshIndex < meshCount; ++meshIndex)
			{
				const Mesh& mesh = perModel.srcModel->GetMeshes().at(meshIndex);
				const Mesh::MeshRange& range = mesh.m_meshRange;

				PerMeshDataGPU* meshDataMap = static_cast<PerMeshDataGPU*>(m_perMeshUniform.Map());
				memcpy(meshDataMap, &mesh.m_meshToModel, sizeof(mesh.m_meshToModel));
				m_perMeshUniform.Unmap();
				m_perMeshUniform.BindVS(PER_DRAW_SLOT);

				uint32_t instancesToDraw{};

				for (const PerMaterial& perMaterial : perModel.perMesh[meshIndex].perMaterial)
				{
					instancesToDraw += static_cast<uint32_t>(perMaterial.instances.size());
				}

				if (!instancesToDraw)
					continue;

				//Keep drawing the same mesh until we have looped through all lights. Updating uniforms is faster than switching VertexBindings.
				for (uint16_t light = 0, sliceTarget = 0; light < numCubemaps; ++light, sliceTarget = light * 6)
				{
					const ShadowMap::CubemapDepthPassBuffer& smUniform
					{
						vp[sliceTarget],
						vp[sliceTarget + 1],
						vp[sliceTarget + 2],
						vp[sliceTarget + 3],
						vp[sliceTarget + 4],
						vp[sliceTarget + 5],
						sliceTarget
					};

					ShadowMap::CubemapDepthPassBuffer* map = static_cast<ShadowMap::CubemapDepthPassBuffer * >(m_depthPassCube_Uniform.Map());
					memcpy(map, &smUniform, sizeof(ShadowMap::CubemapDepthPassBuffer));
					m_depthPassCube_Uniform.Unmap();
					m_depthPassCube_Uniform.BindGS(SHADOW_PASS_CUBE_SLOT);

					if (hasIndexBuffer)
					{
						DX_CALL(s_devcon->DrawIndexedInstanced(range.indexNum, instancesToDraw, range.indexOffset, range.vertexOffset, renderedInstances));
					}
					else
					{
						DX_CALL(s_devcon->DrawInstanced(range.vertexNum, instancesToDraw, range.vertexOffset, renderedInstances));
					}
				}
					renderedInstances += instancesToDraw;
			}
		}

	}


	void OpaqueGroup::RenderInternal()
	{
		uint32_t renderedInstances{};

		m_instanceBuffer.Bind(1);


		for (const PerModel& perModel : m_perModel)
		{
			uint32_t meshCount{ static_cast<uint32_t>(perModel.perMesh.size()) };

			perModel.srcModel->BindVertexBuffer();
			bool hasIndexBuffer{ perModel.srcModel->BindIndexBuffer() };


			for (uint32_t meshIndex = 0; meshIndex < meshCount; ++meshIndex)
			{
				const Mesh& mesh = perModel.srcModel->GetMeshes().at(meshIndex);
				const Mesh::MeshRange& range = mesh.m_meshRange;

				PerMeshDataGPU* meshDataMap = static_cast<PerMeshDataGPU*>(m_perMeshUniform.Map());
				memcpy(meshDataMap, &mesh.m_meshToModel, sizeof(mesh.m_meshToModel));
				m_perMeshUniform.Unmap();
				m_perMeshUniform.BindVS(PER_DRAW_SLOT);

				for (const PerMaterial& perMaterial : perModel.perMesh[meshIndex].perMaterial)
				{
					uint32_t instancesToDraw{ static_cast<uint32_t>(perMaterial.instances.size()) };

					if (!instancesToDraw)
						continue;

					perMaterial.material.Bind();
					
					PerMaterialGPU* materialDataMap = static_cast<PerMaterialGPU*>(m_perMaterialUniform.Map());
					const PerMaterialGPU materialData{ perMaterial.material.flags, perMaterial.material.f_roughness, perMaterial.material.f_metalness };
					memcpy(materialDataMap, &materialData, sizeof(PerMaterialGPU));
					m_perMaterialUniform.Unmap();
					m_perMaterialUniform.BindPS(PER_MATERIAL_SLOT);

					if(hasIndexBuffer)
					{
						DX_CALL(s_devcon->DrawIndexedInstanced(range.indexNum, instancesToDraw, range.indexOffset, range.vertexOffset, renderedInstances));
					}
					else
					{
						DX_CALL(s_devcon->DrawInstanced(range.vertexNum, instancesToDraw, range.vertexOffset, renderedInstances));
					}
					renderedInstances += instancesToDraw;
				}
			}

		}
	}

	bool OpaqueGroup::FindIntersection(const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder)
	{
		return utility::findIntersection(*this, worldRay, outHit, outFinder);
	}

	bool OpaqueGroup::deleteInstance(const Handle<CombinedID>* meshInstances, uint32_t numMeshInstances)
	{
		return utility::deleteInstance(*this, meshInstances, numMeshInstances);
	}

	Handle<OpaqueGroup::PerModel> OpaqueGroup::addModel(const std::shared_ptr<Model>& srcModel)
	{
		Handle<OpaqueGroup::PerModel> outHandle;
		utility::addModel(*this, srcModel, outHandle);

		return outHandle;
	}

	void OpaqueGroup::addMaterial(const OpaqueGroup::Material* materials, Handle<OpaqueGroup::PerModel> model, Handle<OpaqueGroup::Material>* outHandles)
	{
		utility::addMaterial(*this, materials, model, outHandles);
	}


	void OpaqueGroup::addInstances(Handle<OpaqueGroup::PerModel> model, const Handle<OpaqueGroup::Material>* materials, const OpaqueGroup::InstanceData* data, uint32_t numInstances, PerMeshInstance* outIDs)
	{
		utility::addInstances(*this, model, materials, data, numInstances, outIDs);
	}

	bool OpaqueGroup::queryInstanceMaterial(const Handle<CombinedID>* meshInstances, uint32_t numMeshes, Material* outMaterials) const
	{
		return utility::queryInstanceMaterial(*this, meshInstances, numMeshes, outMaterials);
	}

	const std::shared_ptr<Model>& OpaqueGroup::getModelByHandle(Handle<CombinedID> modelHandle) const
	{
		return utility::getModelByHandle(*this, modelHandle);
	}

}


