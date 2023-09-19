#include "pch.h"


#pragma once
#include "include/Render/Renderer/ShadingGroups/DissolutionGroup.h"
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
	DissolutionGroup::PerModel::PerModel(const std::shared_ptr<Model>& srcModel) :
		srcModel(srcModel)
	{
		perMesh.resize(srcModel->GetNumMeshes());
	}


	DissolutionGroup::Material::Material(
		const std::shared_ptr<Texture>& albedo,
		const std::shared_ptr<Texture>& normal,
		const std::shared_ptr<Texture>& roughness,
		const std::shared_ptr<Texture>& metalness,
		const std::shared_ptr<Texture>& noise,
		uint16_t inFlags,
		float f_roughness,
		float f_metalness) :

		albedo(albedo),
		normal(normal),
		roughness(roughness),
		metalness(metalness),
		dissolutionNoise(noise),
		flags(inFlags),
		f_roughness(f_roughness),
		f_metalness(f_metalness)
	{
		if (normal)
			flags |= shading::DissolutionGroup::MaterialFlags::HAS_NORMALS;
		if (roughness)
			flags |= shading::DissolutionGroup::MaterialFlags::HAS_ROUGHNESS;
		if (metalness)
			flags |= shading::DissolutionGroup::MaterialFlags::HAS_METALNESS;
	}

	void DissolutionGroup::Material::Bind() const
	{
		albedo->BindPS(TX_ALBEDO_SLOT);
		dissolutionNoise->BindPS(TX_DIS_NOISE_SLOT);

		if (flags & HAS_NORMALS)
			normal->BindPS(TX_NORMAL_SLOT);

		if (flags & HAS_ROUGHNESS)
			roughness->BindPS(TX_ROUGH_SLOT);

		if (flags & HAS_METALNESS)
			metalness->BindPS(TX_METAL_SLOT);
		
	}

	void DissolutionGroup::Init(uint8_t modelBits, uint8_t materialBits, uint8_t instanceBits)
	{
		IShadingGroup::Init(modelBits, materialBits, instanceBits);

		const D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,  0, 0                        , D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL",   0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,  0, 1 * sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT",  0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,  0, 2 * sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BITANGENT",0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,  0, 3 * sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"UV",       0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,     0, 4 * sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA, 0},

			{"MW",       0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0                                       , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MW",       1, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1 * sizeof(math::Vector4)               , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MW",       2, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 2 * sizeof(math::Vector4)               , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MW",       3, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 3 * sizeof(math::Vector4)               , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"ID",       0, DXGI_FORMAT::DXGI_FORMAT_R32_UINT          , 1, sizeof(math::Matrix)                    , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"TIME",     0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,       1, sizeof(math::Matrix) + sizeof(uint32_t) , D3D11_INPUT_PER_INSTANCE_DATA, 1}
		};



		m_VS.LoadShader(L"DissolutionGroup_VS.cso", ied, static_cast<UINT>(std::size(ied)));
		m_PS.LoadShader(L"DissolutionGroup_PS.cso");
		m_normalDebugGS.LoadShader(L"DissolutionGroup_NormalDebug_GS.cso");
		m_perMeshUniform.CreateGPUBuffer(sizeof(PerMeshDataGPU), 1, nullptr);
		m_perMaterialUniform.CreateGPUBuffer(sizeof(PerMaterialGPU), 1, nullptr);
		m_depthPassCube_Uniform.CreateGPUBuffer(sizeof(ShadowMap::CubemapDepthPassBuffer), 1, nullptr);

		const D3D11_INPUT_ELEMENT_DESC depthIL[] =
		{
			{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,    0, 0                        , D3D11_INPUT_PER_VERTEX_DATA  , 0},
			{"UV",       0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT   ,    0, 4 * sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA  , 0},

			{"MW",       0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0                        , D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{"MW",       1, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1 * sizeof(math::Vector4), D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{"MW",       2, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 2 * sizeof(math::Vector4), D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{"MW",       3, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 3 * sizeof(math::Vector4), D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{"TIME",     0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT,       1,     sizeof(math::Matrix) + sizeof(uint32_t) , D3D11_INPUT_PER_INSTANCE_DATA, 1}

		};

		m_depthPass2D_VS.LoadShader(L"DissolutionGroup_DepthOnly2D_VS.cso", depthIL, static_cast<UINT>(std::size(depthIL)));
		m_depthPass2D_PS.LoadShader(L"DissolutionGroup_DepthOnly2D_PS.cso");

		m_depthPassCube_VS.LoadShader(L"DissolutionGroup_DepthOnlyCube_VS.cso", depthIL, static_cast<UINT>(std::size(depthIL)));
		m_depthPassCube_GS.LoadShader(L"DissolutionGroup_DepthOnlyCube_GS.cso");
		m_depthPassCube_PS.LoadShader(L"DissolutionGroup_DepthOnlyCube_PS.cso");

		m_numInstances = 0;
	}

	void DissolutionGroup::update(float deltaTime)
	{
		if (!m_numInstances)
			return;

		for (PerModel& perModel : m_perModel)
		{
			for (PerMesh& perMesh : perModel.perMesh)
			{
				for (PerMaterial& perMaterial : perMesh.perMaterial)
				{
					uint32_t instances{ static_cast<uint32_t>(perMaterial.instances.size()) };
					for (uint32_t instanceIdx = 0; instanceIdx < instances; ++instanceIdx)
					{
						InstanceData& instanceData = perMaterial.instances.at(instanceIdx);
						instanceData.animationTime += deltaTime;
					}
				}
			}
		}
	}

	void DissolutionGroup::UpdateInstanceBuffer()
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
						const InstanceData& instanceData = perMaterial.instances.at(instanceIdx);
						const math::Matrix& transform = instanceData.handle_modelToWorld.isValid() ? TransformSystem::Get().QueryTransformMatrix(instanceData.handle_modelToWorld) : math::Matrix::Identity;
#ifdef CAMERA_CENTER_WS
						const InstanceDataGPU& src{ transform - cameraPos, instanceData.handle_object.id, instanceData.animationDuration, instanceData.animationTime };
#else
						const InstanceDataGPU& src{ transform, instanceData.handle_object.id, instanceData.animationDuration, instanceData.animationTime };
#endif


						dst[copiedNum++] = src;
					}
				}
			}
		}

		m_instanceBuffer.Unmap();
	}


	void DissolutionGroup::RenderInternal()
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

					perMaterial.material.Bind();

					PerMaterialGPU* materialDataMap = static_cast<PerMaterialGPU*>(m_perMaterialUniform.Map());
					const PerMaterialGPU materialData{ perMaterial.material.flags, perMaterial.material.f_roughness, perMaterial.material.f_metalness };
					memcpy(materialDataMap, &materialData, sizeof(PerMaterialGPU));
					m_perMaterialUniform.Unmap();
					m_perMaterialUniform.BindPS(PER_MATERIAL_SLOT);

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
	}

	void DissolutionGroup::Render()
	{
		if (m_numInstances == 0)
			return;

		m_PS.Bind();
		m_VS.Bind();
		RenderInternal();
	}

	void DissolutionGroup::RenderDebugNormals()
	{
		if (!ShouldDebugNormals())
			return;

		m_VS.Bind();
		m_normalDebugGS.Bind();
		m_instanceBuffer.Bind(1);
		RenderInternal();
	}

	void DissolutionGroup::RenderDepth2DOnly()
	{
		if (!m_numInstances)
			return;

		m_depthPass2D_VS.Bind();
		m_depthPass2D_PS.Bind();
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

			
				for (const PerMaterial& perMaterial : perModel.perMesh[meshIndex].perMaterial)
				{
					uint32_t instancesToDraw = static_cast<uint32_t>(perMaterial.instances.size());

					perMaterial.material.dissolutionNoise->BindPS(TX_DIS_NOISE_SLOT);

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
	}

	void DissolutionGroup::RenderDepthCubemapsOnly(const math::Matrix* vp, uint16_t numCubemaps)
	{
		if (!m_numInstances)
			return;

		uint32_t renderedInstances{};

		m_depthPassCube_VS.Bind();
		m_depthPassCube_GS.Bind();
		m_depthPassCube_PS.Bind();
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
					uint32_t instancesToDraw = static_cast<uint32_t>(perMaterial.instances.size());

					perMaterial.material.dissolutionNoise->BindPS(TX_DIS_NOISE_SLOT);
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

						ShadowMap::CubemapDepthPassBuffer* map = static_cast<ShadowMap::CubemapDepthPassBuffer*>(m_depthPassCube_Uniform.Map());
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

	}


	Handle<DissolutionGroup::PerModel> DissolutionGroup::addModel(const std::shared_ptr<Model>& srcModel)
	{
		Handle<DissolutionGroup::PerModel> outHandle;
		utility::addModel(*this, srcModel, outHandle);
		return outHandle;
	}

	void DissolutionGroup::addMaterial(const Material* materials, Handle<PerModel> model, Handle<Material>* outHandles)
	{
		utility::addMaterial(*this, materials, model, outHandles);
	}

	void DissolutionGroup::addInstances(Handle<PerModel> model, const Handle<Material>* materials, const InstanceData* data, uint32_t numInstances, PerMeshInstance* outIDs)
	{
		utility::addInstances(*this, model, materials, data, numInstances, outIDs);
	}

	const DissolutionGroup::InstanceData& DissolutionGroup::getInstanceData(PerMeshInstance targetInstance) const
	{
		return utility::getInstanceData<DissolutionGroup, InstanceData>(*this, targetInstance);
	}

	bool DissolutionGroup::deleteInstance(const Handle<CombinedID>* instanceHandles, uint32_t numMeshes)
	{
		return utility::deleteInstance(*this, instanceHandles, numMeshes);
	}

	const std::shared_ptr<Model>& DissolutionGroup::getModelByHandle(Handle<CombinedID> handle) const
	{
		return utility::getModelByHandle(*this, handle);
	}

	bool DissolutionGroup::FindIntersection(const Ray& worldRay, math::HitResult& outHit, InstanceFinder& outFinder)
	{
		return utility::findIntersection(*this, worldRay, outHit, outFinder);
	}
}
