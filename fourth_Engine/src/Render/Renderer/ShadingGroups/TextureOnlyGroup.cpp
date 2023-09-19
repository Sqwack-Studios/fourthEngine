#include "pch.h"
#pragma once
#include "include/Render/Renderer/ShadingGroups/TextureOnlyGroup.h"
#include "include/Render/Renderer/D3D11Headers.h"
#include "include/Render/Model.h"
#include "include/Managers/TextureManager.h"
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
	TextureOnlyGroup::PerModel::PerModel(std::shared_ptr<Model> model) :
	srcModel(model)
	{
		perMesh.resize(model->GetNumMeshes());
	}

	void TextureOnlyGroup::Material::Bind() const
	{
		albedo->BindPS(TX_ALBEDO_SLOT);
	}


	void TextureOnlyGroup::Init(uint8_t modelBits, uint8_t materialBits, uint8_t instanceBits)
	{
		IShadingGroup::Init(modelBits, materialBits, instanceBits);

		const D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			//VertexData
			{"POSITION",      0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT   ,    0,  0                        , D3D11_INPUT_PER_VERTEX_DATA,   0},
			{"NORMAL"  ,      0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT   ,    0,  1 * sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA,   0},
			{"UV"      ,      0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT      ,    0,  4 * sizeof(math::Vector3), D3D11_INPUT_PER_VERTEX_DATA,   0},
			//InstanceData															     
			{"MW"      ,      0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,    1,  0                        , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MW"      ,      1, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,    1,  1 * sizeof(math::Vector4), D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MW"      ,      2, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,    1,  2 * sizeof(math::Vector4), D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"MW"      ,      3, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,    1,  3 * sizeof(math::Vector4), D3D11_INPUT_PER_INSTANCE_DATA, 1}
		};

		m_vs.LoadShader(L"TextureOnly_VS.cso", ied, (UINT)std::size(ied));
		m_ps.LoadShader(L"TextureOnly_PS.cso");

		m_normalDebugVS.LoadShader(L"TextureOnly_NormalDebug_VS.cso", ied, (UINT)std::size(ied));
		m_debugGS.LoadShader(L"TextureOnly_NormalDebug_GS.cso");

		//m_materialUniform.CreateGPUBuffer(1, nullptr);
		m_meshUniform.CreateGPUBuffer(sizeof(PerMeshData), 1, nullptr);
		m_numInstances = 0;
	}
	void TextureOnlyGroup::UpdateInstanceBuffer()
	{
		if (m_perModel.empty())
			return;

		m_instanceBuffer.CreateGPUBuffer(sizeof(InstanceDataGPU), m_numInstances, nullptr);
		InstanceDataGPU* dst = static_cast<InstanceDataGPU*>(m_instanceBuffer.Map());

		uint32_t copiedNum{};

//#ifdef CAMERA_CENTER_WS
//		math::Matrix cameraPos{};
//		cameraPos.Translation(CameraManager::GetActiveCamera().GetPosition());
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
//						const InstanceDataGPU& src{ transform - cameraPos };
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

	void TextureOnlyGroup::Render()
	{
		if (m_numInstances == 0)
			return;

		m_ps.Bind();
		m_vs.Bind();
		m_instanceBuffer.Bind(1);
		RenderInternal();
	}

	void TextureOnlyGroup::RenderDebugNormals()
	{
		if (!ShouldDebugNormals())
			return;

		m_debugGS.Bind();
		m_normalDebugVS.Bind();
		m_instanceBuffer.Bind(1);
		RenderInternal();
	}

	void TextureOnlyGroup::RenderInternal()
	{

		uint32_t renderedInstances{};

		for (const PerModel& perModel : m_perModel)
		{
			uint32_t meshCount{ static_cast<uint32_t>(perModel.perMesh.size()) };

			perModel.srcModel->BindVertexBuffer();
			perModel.srcModel->BindIndexBuffer();

			for (uint32_t meshIndex = 0; meshIndex < meshCount; ++meshIndex)
			{
				const Mesh& mesh = perModel.srcModel->GetMeshes().at(meshIndex);
				const Mesh::MeshRange& range = mesh.m_meshRange;

				void* meshDataMap = m_meshUniform.Map();
				memcpy(meshDataMap, &mesh.m_meshToModel, sizeof(mesh.m_meshToModel));
				m_meshUniform.Unmap();
				m_meshUniform.BindVS(PER_DRAW_SLOT);

				for (const PerMaterial& perMaterial : perModel.perMesh[meshIndex].perMaterial)
				{
					uint32_t instancesToDraw{ static_cast<uint32_t>(perMaterial.instances.size()) };

					perMaterial.material.Bind();
					DX_CALL(s_devcon->DrawIndexedInstanced(range.indexNum, instancesToDraw, range.indexOffset, range.vertexOffset, renderedInstances));
					renderedInstances += instancesToDraw;
				}
			}

		}
	}

	bool TextureOnlyGroup::FindIntersection(const Ray& ray, math::HitResult& outHit, InstanceFinder& outFinder)
	{
		return utility::findIntersection(*this, ray, outHit, outFinder);
	}


	Handle<TextureOnlyGroup::PerModel> TextureOnlyGroup::addModel(const std::shared_ptr<Model>& srcModel)
	{
		Handle<PerModel> outPerModel;

		utility::addModel(*this, srcModel, outPerModel);

		return outPerModel;
	}

	void TextureOnlyGroup::addMaterial(const Material* materials, Handle<PerModel> model, Handle<Material>* outHandles)
	{
		utility::addMaterial(*this, materials, model, outHandles);
	}

	void TextureOnlyGroup::addInstances(Handle<PerModel> model, const Handle<Material>* materials, const InstanceData* data, uint32_t numInstances, PerMeshInstance* outIDs)
	{
		utility::addInstances(*this, model, materials, data, numInstances, outIDs);
	}

}