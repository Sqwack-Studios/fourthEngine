#include "pch.h"

#pragma once
#include "include/Systems/DecalSystem.h"
#include "include/Render/Renderer/D3DRenderer.h"
#include "include/Managers/ModelManager.h"
#include "include/Render/Model.h"
#include "Shaders/RegisterSlots.hlsli"
#include "include/Systems/MeshSystem.h"

namespace fth
{
	DecalSystem::DecalSystem(){}
	DecalSystem::~DecalSystem(){}

	void DecalSystem::Init()
	{
		const D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			{"POSITION",    0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT,     0,   0                                                   , D3D11_INPUT_PER_VERTEX_DATA,   0},
			{"DW",          0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   0                                                   , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"DW",          1, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,       sizeof(math::Vector4)                           , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"DW",          2, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   2 * sizeof(math::Vector4)                           , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"DW",          3, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   3 * sizeof(math::Vector4)                           , D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"WD",          0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   0 * sizeof(math::Vector4) +     sizeof(math::Matrix), D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"WD",          1, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   1 * sizeof(math::Vector4) +     sizeof(math::Matrix), D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"WD",          2, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   2 * sizeof(math::Vector4) +     sizeof(math::Matrix), D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"WD",          3, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   3 * sizeof(math::Vector4) +     sizeof(math::Matrix), D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"COLOR",       0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT,  1,   0 * sizeof(math::Vector4) + 2 * sizeof(math::Matrix), D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"PID",         0, DXGI_FORMAT::DXGI_FORMAT_R32_UINT          ,  1,   1 * sizeof(math::Vector4) + 2 * sizeof(math::Matrix), D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};

		m_decalVS.LoadShader(L"Decal_VS.cso", ied, (UINT)std::size(ied));
		m_decalPS.LoadShader(L"Decal_PS.cso");

		m_perMaterialUniform.CreateGPUBuffer(sizeof(PerMaterialGPU), 1, nullptr);
		m_numInstances = 0;
	}

	void DecalSystem::Shutdown()
	{

	}

	void DecalSystem::addDecal(const DecalMaterial& material, Handle<math::Matrix> decalToModel, Handle<math::Matrix> parentID, Handle<RenderableObject> parentObjectID, const math::Color& color)
	{
		PerMaterial* perMaterialPtr{};

		
		for (PerMaterial& perMaterial : m_perMaterial)
		{
			if (perMaterial.material == material)
			{
				perMaterialPtr = &perMaterial;
				break;
			}
		}

		if (!perMaterialPtr)
		{
			perMaterialPtr = &m_perMaterial.emplace_back(material);
		}
		

		DecalInstanceCPU& instance = perMaterialPtr->instances.emplace_back(color, decalToModel, parentID, parentObjectID);
		++m_numInstances;
	}

	void DecalSystem::update()
	{
		if (!m_numInstances)
			return;


		uint32_t copiedNum{};


		m_instanceBuffer.CreateGPUBuffer(sizeof(DecalInstanceGPU), m_numInstances, nullptr);

		DecalInstanceGPU* dst = static_cast<DecalInstanceGPU*>(m_instanceBuffer.Map());


#ifdef CAMERA_CENTER_WS

		math::Matrix cameraPos{};
		cameraPos.Translation(CameraManager::GetActiveCamera().GetPosition());
#endif

		for (const PerMaterial& perMaterial : m_perMaterial)
		{
			for (const DecalInstanceCPU& perInstance : perMaterial.instances)
			{
				math::Matrix transform = math::Matrix::Identity;

				if (perInstance.decalToModelID.isValid())
					transform = TransformSystem::Get().QueryTransformMatrix(perInstance.decalToModelID);

				if(perInstance.parentID.isValid())
				{
					transform *= TransformSystem::Get().QueryTransformMatrix(perInstance.parentID);
				}

#ifdef CAMERA_CENTER_WS

					transform -= cameraPos;

#endif
				math::Matrix invTransform;
				math::InvertOrthogonalMatrix(transform, invTransform);

				dst[copiedNum++] = { transform, invTransform, perInstance.color, perInstance.parentObjectID.id };
			}
		}
		m_instanceBuffer.Unmap();
	}

	void DecalSystem::draw()
	{
		if (!m_numInstances)
			return;

		m_decalVS.Bind();
		m_decalPS.Bind();
		m_instanceBuffer.Bind(1);

		const Model& cubeModel = *ModelManager::Get().GetUnitCube();
		cubeModel.BindVertexBuffer();
		bool hasIndexBuffer{ cubeModel.BindIndexBuffer() };

		const Mesh::MeshRange range = cubeModel.GetMeshes()[0].m_meshRange;
		uint32_t renderedInstances{};

		{
			renderer::D3DRenderer& renderer{ renderer::D3DRenderer::Get() };
			renderer.getGBufferNormalsCopy().BindPS(TX_DECAL_NORMAL_GB_SLOT);
			renderer.getDepthBufferCopy().BindPS(TX_DECAL_DEPTH_SLOT);
			renderer.getGBufferObjectIDCopy().BindPS(TX_DECAL_OBJECTID_GB_SLOT);
			renderer.BindRasterizerState(renderer::RSTYPES::RSTYPE_NOCULL_SOLID);
		}


		for (const PerMaterial& perMaterial : m_perMaterial)
		{
			uint32_t instancesToDraw{ static_cast<uint32_t> (perMaterial.instances.size())};
		    
			perMaterial.material.tx_normal->BindPS(TX_DECAL_NORMAL_ALPHA_SLOT);
			PerMaterialGPU materialData;
			
			if (perMaterial.material.invertNormals)
			{
				materialData.invertNormal_buildBlue |= 1 << 0;
			}
			else
			{
				materialData.invertNormal_buildBlue &= ~(1 << 0);

			}


			if (perMaterial.material.buildBlueNormals)
			{
				materialData.invertNormal_buildBlue |= 1 << 1;

			}
			else
			{
				materialData.invertNormal_buildBlue &= ~(1 << 1);

			}

			PerMaterialGPU* dst = static_cast<PerMaterialGPU*>(m_perMaterialUniform.Map());

			dst[0] = materialData;
			m_perMaterialUniform.Unmap();

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