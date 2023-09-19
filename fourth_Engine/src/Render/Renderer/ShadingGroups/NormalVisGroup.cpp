#include "pch.h"

#pragma once
#include "include/Render/Renderer/ShadingGroups/NormalVisGroup.h"
#include "include/Math/Ray.h"
#include "include/Render/Model.h"
#include "include/Render/Renderer/D3D11Headers.h"
#include "include/Systems/TransformSystem.h"
#include "Shaders/RegisterSlots.hlsli"
#include "include/Managers/CameraManager.h"


namespace fth
{
	extern ID3D11DeviceContext4* s_devcon;
	namespace shading
	{
		NormalVisGroup::PerModel::PerModel(const std::shared_ptr<Model>& srcModel):
			srcModel(srcModel)
		{
			perMesh.resize(srcModel->GetNumMeshes());

			for (auto& perMesh : this->perMesh)
			{
				perMesh.perMaterial.resize(1);
			}
		}

		void NormalVisGroup::Init(uint8_t modelBits, uint8_t materialBits, uint8_t instanceBits)
		{
			IShadingGroup::Init(modelBits, materialBits, instanceBits);

			const D3D11_INPUT_ELEMENT_DESC ied[] =
			{
				{"POSITION",    0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,     0,   0                        , D3D11_INPUT_PER_VERTEX_DATA,   0},
				{"NORMAL",      0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,     0,       sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA,   0},
				{"MW",          0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   0                        , D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MW",          1, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,       sizeof(math::Vector4), D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MW",          2, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   2 * sizeof(math::Vector4), D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MW",          3, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   3 * sizeof(math::Vector4), D3D11_INPUT_PER_INSTANCE_DATA, 1}
			};

			m_vs.LoadShader(L"Instanced_NormalVis_VS.cso", ied, (UINT)std::size(ied));
			m_ps.LoadShader(L"Instanced_NormalVis_PS.cso");
			m_debugNormalsGS.LoadShader(L"NormalVis_NormalDebug_GS.cso");

			m_perMeshUniform.CreateGPUBuffer(sizeof(PerMeshData), 1, nullptr);
			m_numInstances = 0;
		}



		void NormalVisGroup::UpdateInstanceBuffer()
		{
			if (m_perModel.empty())
				return;

			m_instanceBuffer.CreateGPUBuffer(sizeof(InstanceDataGPU), m_numInstances, nullptr);
			InstanceDataGPU* dst = static_cast<InstanceDataGPU*>(m_instanceBuffer.Map());

			uint32_t copiedNum{};

//#ifdef CAMERA_CENTER_WS
//			math::Matrix cameraPos{};
//			cameraPos.Translation(CameraManager::GetActiveCamera().GetPosition());
//#endif

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
//							const InstanceDataGPU& src{ transform - cameraPos };
//#else
							const InstanceDataGPU& src{ transform };
//#endif
							dst[copiedNum++] = src;
						}
					}


					
				}
			}
			m_instanceBuffer.Unmap();
		}



		bool NormalVisGroup::FindIntersection(const Ray& ray, math::HitResult& outHit, InstanceFinder& outInstanceFinder)
		{
			return utility::findIntersection(*this, ray, outHit, outInstanceFinder);
		}


		void NormalVisGroup::Render()
		{
			if (m_perModel.empty())
				return;

			m_vs.Bind();
			m_ps.Bind();
			m_instanceBuffer.Bind(1);
			RenderInternal();
		}

		void NormalVisGroup::RenderDebugNormals()
		{
			if (!ShouldDebugNormals())
				return;

			m_vs.Bind();
			m_debugNormalsGS.Bind();
			m_instanceBuffer.Bind(1);
			RenderInternal();
		}

		void NormalVisGroup::RenderInternal()
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

					for(const PerMaterial& perMaterial : perMesh.perMaterial)
					{

						uint32_t instancesToRender{ static_cast<uint32_t>(perMaterial.instances.size()) };

						void* meshDataMap = m_perMeshUniform.Map();
						memcpy(meshDataMap, &srcMesh.m_meshToModel, sizeof(srcMesh.m_meshToModel));
						m_perMeshUniform.Unmap();
						m_perMeshUniform.BindVS(PER_DRAW_SLOT);

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


		Handle<NormalVisGroup::PerModel> NormalVisGroup::addModel(const std::shared_ptr<Model>& srcModel)
		{
			Handle<PerModel> outHandle;

			utility::addModel(*this, srcModel, outHandle);
			return outHandle;
		}

		void NormalVisGroup::addMaterial(const Material* materials, Handle<PerModel> model, Handle<Material>* outHandles)
		{
			utility::addMaterial(*this, materials, model, outHandles);
		}
		void NormalVisGroup::addInstances(Handle<PerModel> model, const Handle<Material>* materials, const InstanceData* data, uint32_t numInstances, PerMeshInstance* outIDs)
		{
			utility::addInstances(*this, model, materials, data, numInstances, outIDs);
		}

	}
}