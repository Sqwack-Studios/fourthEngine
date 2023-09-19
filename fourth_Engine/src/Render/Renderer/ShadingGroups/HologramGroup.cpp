#include "pch.h"
#pragma once
#include "include/Render/Renderer/ShadingGroups/HologramGroup.h"
#include "include/Render/Renderer/D3DRenderer.h"
#include "include/Math/Ray.h"
#include "include/Render/Model.h"
#include "include/Systems/TransformSystem.h"
#include "Shaders/RegisterSlots.hlsli"



namespace fth
{
	extern ID3D11DeviceContext4* s_devcon;
	namespace shading
	{
		HologramGroup::PerModel::PerModel(const std::shared_ptr<Model>& srcModel):
			srcModel(srcModel)
		{
			perMesh.resize(srcModel->GetNumMeshes());

			for (auto& perMesh : this->perMesh)
				perMesh.perMaterial.resize(1);
		}

		void HologramGroup::Init(uint8_t modelBits, uint8_t materialBits, uint8_t instanceBits)
		{
			IShadingGroup::Init(modelBits, materialBits, instanceBits);

			const D3D11_INPUT_ELEMENT_DESC ied[] =
			{
				{"POSITION",    0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,     0,   0                        , D3D11_INPUT_PER_VERTEX_DATA,   0},
				{"NORMAL",      0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,     0,       sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA,   0},
				{"MW",          0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   0                        , D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MW",          1, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,       sizeof(math::Vector4), D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MW",          2, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   2 * sizeof(math::Vector4), D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"MW",          3, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   3 * sizeof(math::Vector4), D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{"COLOR",       0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,     1,   4 * sizeof(math::Vector4),   D3D11_INPUT_PER_INSTANCE_DATA, 1}
			};

			m_vs.LoadShader(L"Instanced_Hologram_VS.cso", ied, (UINT)std::size(ied));
			m_ps.LoadShader(L"Instanced_Hologram_PS.cso");
			m_hs.LoadShader(L"Instanced_Hologram_HS.cso");
			m_ds.LoadShader(L"Instanced_Hologram_DS.cso");
			m_gs.LoadShader(L"Instanced_Hologram_GS.cso");

			m_perMeshUniform.CreateGPUBuffer(sizeof(PerMeshData), 1, nullptr);

			m_normalDebugGS.LoadShader(L"Hologram_NormalDebug_GS.cso");
			m_numInstances = 0;
		}

		void HologramGroup::Render()
		{
			if (m_perModel.empty())
				return;

			m_vs.Bind(); 
			m_ps.Bind();
			m_hs.Bind();
			m_ds.Bind();
			m_gs.Bind(); 
			m_instanceBuffer.Bind(1);
			renderer::D3DRenderer::Get().SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			RenderInternal();
			
		}

		void HologramGroup::RenderDebugNormals()
		{
			if (!ShouldDebugNormals())
				return;

			renderer::D3DRenderer::Get().BindPSNormalDebug();
			m_normalDebugGS.Bind();
			RenderInternal();
		}

		void HologramGroup::RenderInternal()
		{
			uint32_t instancesRendered{};

			for (const PerModel& perModel : m_perModel)
			{
				perModel.srcModel->BindVertexBuffer();
				bool hasIndexBuffer{ perModel.srcModel->BindIndexBuffer() };
				uint32_t meshCount{ static_cast<uint32_t>(perModel.perMesh.size()) };
				for (uint32_t meshIdx = 0; meshIdx < meshCount; ++meshIdx)
				{
					const Mesh& srcMesh = perModel.srcModel->GetMeshes()[meshIdx];
					const Mesh::MeshRange& range = srcMesh.m_meshRange;

					PerMeshData* dst = static_cast<PerMeshData*>(m_perMeshUniform.Map());
					memcpy(dst, &srcMesh.m_meshToModel, sizeof(srcMesh.m_meshToModel));
					m_perMeshUniform.Unmap();
					m_perMeshUniform.BindVS(PER_DRAW_SLOT);

					for (const PerMaterial& perMaterial : perModel.perMesh[meshIdx].perMaterial)
					{
						uint32_t numInstances{ static_cast<uint32_t>(perMaterial.instances.size()) };

						if (hasIndexBuffer)
						{
						     DX_CALL(s_devcon->DrawIndexedInstanced(range.indexNum, numInstances, range.indexOffset, range.vertexOffset, instancesRendered));
						}
						else
						{
							DX_CALL(s_devcon->DrawInstanced(range.vertexNum, numInstances, range.vertexOffset, instancesRendered));
						}
						instancesRendered += numInstances;
					}

				}
			}
		}

		void HologramGroup::UpdateInstanceBuffer()
		{
			if (m_perModel.empty())
				return;

			m_instanceBuffer.CreateGPUBuffer(sizeof(InstanceDataGPU), m_numInstances, nullptr);
			InstanceDataGPU* dst = static_cast<InstanceDataGPU*>(m_instanceBuffer.Map());
			uint32_t instancesCopied{};

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
						uint32_t numInstances{ static_cast<uint32_t>(perMaterial.instances.size()) };

						for (uint32_t instanceIdx = 0; instanceIdx < numInstances; ++instanceIdx)
						{
							const InstanceData& instanceData = perMaterial.instances.at(instanceIdx);
							const math::Matrix& transform = instanceData.handle_modelToWorld.isValid() ? TransformSystem::Get().QueryTransformMatrix(instanceData.handle_modelToWorld)
								: math::Matrix::Identity;
#ifdef CAMERA_CENTER_WS
							const InstanceDataGPU& src{
	                             transform - cameraPos,
	                             perMaterial.instances[instanceIdx].color
							};
#else
							const InstanceDataGPU& src{
	                             transform,
	                             instanceData.color
						    };
#endif


							dst[instancesCopied++] = src;
						}
					}

				}
			}
			m_instanceBuffer.Unmap();
		}


		bool HologramGroup::FindIntersection(const Ray& ray, math::HitResult& outHit, InstanceFinder& outInstanceFinder)
		{
			return utility::findIntersection(*this, ray, outHit, outInstanceFinder);
		}

		Handle<HologramGroup::PerModel> HologramGroup::addModel(const std::shared_ptr<Model>& srcModel)
		{
			Handle<PerModel> outHandle;
			utility::addModel(*this, srcModel, outHandle);
			return outHandle;
		}

		void HologramGroup::addMaterial(const Material* materials, Handle<PerModel> model, Handle<Material>* outHandles)
		{
			utility::addMaterial(*this, materials, model, outHandles);
		}
		void HologramGroup::addInstances(Handle<PerModel> model, const Handle<Material>*materials, const InstanceData * data, uint32_t numInstances, PerMeshInstance * outIDs)
		{
			utility::addInstances(*this, model, materials, data, numInstances, outIDs);
		}



	}
}