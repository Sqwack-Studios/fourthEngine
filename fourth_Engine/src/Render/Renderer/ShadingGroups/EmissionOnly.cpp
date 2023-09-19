#include "pch.h"
#pragma once
#include "include/Render/Renderer/ShadingGroups/EmissionOnly.h"
#include "include/Render/Renderer/D3D11Headers.h"
#include "include/Render/Model.h"
#include "include/Math/Ray.h"
#include "include/Systems/TransformSystem.h"
#include "Shaders/RegisterSlots.hlsli"
#include "include/Managers/CameraManager.h"

namespace fth
{
	extern ID3D11DeviceContext4* s_devcon;
}
namespace fth::shading
{
	EmissionOnly::PerModel::PerModel(const std::shared_ptr<Model>& srcModel):
		srcModel(srcModel)
	{
		perMesh.resize(srcModel->GetNumMeshes());
		for (PerMesh& perMesh : this->perMesh)
		{
			perMesh.perMaterial.resize(1);
		}
	}

	void EmissionOnly::Init(uint8_t modelBits, uint8_t materialBits, uint8_t instanceBits)
	{
		IShadingGroup::Init(modelBits, materialBits, instanceBits);
		const D3D11_INPUT_ELEMENT_DESC ed[] =
		{
			{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT   , 0, 0                                                , D3D11_INPUT_PER_VERTEX_DATA  , 0},
			{"NORMAL",   0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT   , 0, 1 * sizeof(math::Vector3)                        , D3D11_INPUT_PER_VERTEX_DATA  , 0},
			{"MW",       0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0                                                , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MW",       1, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 1 * sizeof(math::Vector4)                        , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MW",       2, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 2 * sizeof(math::Vector4)                        , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MW",       3, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 3 * sizeof(math::Vector4)                        , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"COLOR",    0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT   , 1, 4 * sizeof(math::Vector4)                        , D3D11_INPUT_PER_INSTANCE_DATA, 1}

		};
	 
		m_VS.LoadShader(L"EmissiveOnly_VS.cso", ed, (UINT)std::size(ed));
		m_PS.LoadShader(L"EmissiveOnly_PS.cso");
		m_normalDebugGS.LoadShader(L"EmissiveOnly_NormalDebug_GS.cso");

		m_meshUniform.CreateGPUBuffer(sizeof(PerMeshData), 1, nullptr);
		m_numInstances = 0;
		
	}

	void EmissionOnly::UpdateInstanceBuffer()
	{
		if (m_numInstances == 0)
			return;

		m_instanceData.CreateGPUBuffer(sizeof(InstanceDataGPU), m_numInstances, nullptr);

		InstanceDataGPU* dst = reinterpret_cast<InstanceDataGPU*>(m_instanceData.Map());

//#ifdef CAMERA_CENTER_WS
//		math::Matrix cameraPos{};
//		cameraPos.Translation(CameraManager::GetActiveCamera().GetPosition());
//#endif

		uint32_t updatedInstances{};
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
						const math::Matrix& transform = instanceData.handle_modelToWorld.isValid() ? TransformSystem::Get().QueryTransformMatrix(instanceData.handle_modelToWorld)
							: math::Matrix::Identity;
//#ifdef CAMERA_CENTER_WS
//						const InstanceDataGPU& src{ 
//							transform - cameraPos,
//							perMaterial.instances[instanceIdx].emissionColor
//						};
//#else
						const InstanceDataGPU& src{ 
							transform,
							instanceData.emissionColor
						};
//#endif

						dst[updatedInstances++] = src;
					}
				}

			}
		}

		m_instanceData.Unmap();
	}

	void EmissionOnly::Render()
	{
		if (m_numInstances == 0)
			return;

		m_PS.Bind();
		m_VS.Bind();
		m_instanceData.Bind(1);
		RenderInternal();
	}

	void EmissionOnly::RenderDebugNormals()
	{
		if (!ShouldDebugNormals())
			return;

		m_VS.Bind();
		m_normalDebugGS.Bind();
		m_instanceData.Bind(1);
		RenderInternal();
	}

	void EmissionOnly::RenderInternal()
	{
		uint32_t instancesRendered{};
		for (const PerModel& perModel : m_perModel)
		{
			uint32_t meshIdx{};
			perModel.srcModel->BindVertexBuffer();
			bool hasIndexBuffer{ perModel.srcModel->BindIndexBuffer() };

			for (const PerMesh& perMesh : perModel.perMesh)
			{
				const Mesh& srcMesh = perModel.srcModel->GetMeshes()[meshIdx];
				const Mesh::MeshRange& range = srcMesh.m_meshRange;

				for (const PerMaterial& perMaterial : perMesh.perMaterial)
				{
					uint32_t instancesToRender{ static_cast<uint32_t>(perMaterial.instances.size()) };

					void* meshDataMap = m_meshUniform.Map();
					memcpy(meshDataMap, &srcMesh.m_meshToModel, sizeof(srcMesh.m_meshToModel));
					m_meshUniform.Unmap();
					m_meshUniform.BindVS(PER_DRAW_SLOT);

					if (hasIndexBuffer)
					{
						DX_CALL(s_devcon->DrawIndexedInstanced(range.indexNum, instancesToRender, range.indexOffset, range.vertexOffset, instancesRendered));
					}
					else
						DX_CALL(s_devcon->DrawInstanced(range.vertexNum, instancesToRender, range.vertexOffset, instancesRendered));

					++meshIdx;
					instancesRendered += instancesToRender;
				}
			}
		}
	}

	Handle<EmissionOnly::PerModel> EmissionOnly::addModel(const std::shared_ptr<Model>& srcModel)
	{
		Handle<PerModel> out;

		utility::addModel(*this, srcModel, out);
		return out;
	}

	void EmissionOnly::addMaterial(const Material* materials, Handle<PerModel> modelIdx, Handle<Material>* outHandles)
	{
		utility::addMaterial(*this, materials, modelIdx, outHandles);
	}

	void EmissionOnly::addInstances(Handle<PerModel> modelIdx, const Handle<Material>*matIdx, const InstanceData * data, uint32_t numInstances, PerMeshInstance * outIDs)
	{
		utility::addInstances(*this, modelIdx, matIdx, data, numInstances, outIDs);
	}



	bool EmissionOnly::FindIntersection(const Ray& ray, math::HitResult& outHit, InstanceFinder& outInstanceFinder)
	{
		return utility::findIntersection(*this, ray, outHit, outInstanceFinder);
	}



}