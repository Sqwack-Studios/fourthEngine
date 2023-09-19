#include "pch.h"
#pragma once
#include "include/Systems/ParticleSystem.h"
#include "include/Render/Renderer/D3DRenderer.h"
#include "include/Managers/CameraManager.h"
#include "include/Managers/ModelManager.h"
#include "include/Render/Model.h"
#include "../Shaders/RegisterSlots.hlsli"
namespace fth
{
	extern ID3D11DeviceContext4* s_devcon;

	ParticleSystem::ParticleSystem(){}
	ParticleSystem::~ParticleSystem(){}

	void ParticleSystem::Init()
	{
		const D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			{"COLOR",    0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT, 1,  0,  D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT   , 1,  sizeof(math::Vector4),  D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"ROTATION", 0, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT         , 1,  sizeof(math::Vector4) +     sizeof(math::Vector3),  D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"SPEED",    0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT   , 1,  sizeof(math::Vector4) +     sizeof(math::Vector3) + sizeof(float),  D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"TIME",     0, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT         , 1,  sizeof(math::Vector4) + 2 * sizeof(math::Vector3) + sizeof(float),  D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"SIZE",     0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT      , 1,  sizeof(math::Vector4) + 2 * sizeof(math::Vector3) + 2 * sizeof(float),  D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"TILE",     0, DXGI_FORMAT::DXGI_FORMAT_R32_UINT          , 1,  sizeof(math::Vector4) + 2 * sizeof(math::Vector3) + sizeof(math::Vector2) + 2 * sizeof(float),  D3D11_INPUT_PER_INSTANCE_DATA, 1}
		
		};
		m_smokeVS.LoadShader(L"_6Way_SmokeEffect_VS.cso", ied, (UINT)std::size(ied));
		m_smokePS.LoadShader(L"_6Way_SmokeEffect_PS.cso");
		m_atlasUniform.CreateGPUBuffer(sizeof(tiledAtlasUniformGPU), 1, nullptr);
		
		ringBufferRange initialRange;
		initialRange.number = initialRange.offset = initialRange.expired = 0;
		initialRange.particleArgs.indexCountPerInstance = 6;
		initialRange.particleArgs.instanceCount = 0;
		initialRange.particleArgs.startIndexLocation = 0;
		initialRange.particleArgs.baseVertexLocation = 0;
		initialRange.particleArgs.startInstanceLocation = 0;

		m_computeUpdateParticles.loadShader(L"update_Particles_RingBuffer_CS.cso");
		m_computeUpdateRangeBuffer.loadShader(L"update_RangeBuffer_CS.cso");


		m_particlesRingBuffer.init(MAX_GPU_PARTICLES, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, 0, 0, nullptr);


		renderer::ViewBufferDsc bfDsc;
		bfDsc.firstElement = 0;

#ifdef PARTICLES_LIGHTNING_SPHERES
		drawArgs diffuseDrawArgs { ModelManager::GetUnitSphereSmooth()->GetTriangleIndexBuffer().size() * 3, 0, 0, 0, 0};
		initialRange.sphereDrawArgs = diffuseDrawArgs;

#endif
		uint32_t rangeCount{ sizeof(ringBufferRange) / sizeof(uint32_t) };
		bfDsc.numElements = rangeCount;
		m_particlesRangeBuffer.init(rangeCount, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS, &initialRange);
		m_particlesRangeUAV.CreateBufferUAV(m_particlesRangeBuffer.operator->(), DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, bfDsc, D3D11_BUFFER_UAV_FLAG_RAW);
		m_particlesRangeSRV.CreateRawShaderResourceBuffer(m_particlesRangeBuffer.operator->(), DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS, bfDsc);

		bfDsc.numElements = MAX_GPU_PARTICLES;
		m_particlesRingSRV.CreateShaderResourceBuffer(m_particlesRingBuffer.operator->(), DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, bfDsc);
		m_particlesRingUAV.CreateBufferUAV(m_particlesRingBuffer.operator->(), DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, bfDsc, 0);

		m_gpuParticlesPS.LoadShader(L"RenderGPU_Particles_PS.cso");
		m_gpuParticlesVS.LoadShader(L"RenderGPU_Particles_VS.cso", nullptr, 0);

		const D3D11_INPUT_ELEMENT_DESC ied2[] =
		{
			{"LPOSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0,  0,  D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		m_gpuParticlesDiffusePS.LoadShader(L"GPU_Particles_Diffuse_PS.cso");
		m_gpuParticlesDiffuseVS.LoadShader(L"GPU_Particles_Diffuse_VS.cso", ied2, (UINT)std::size(ied2));

	}


	void ParticleSystem::Shutdown()
	{

	}

	void ParticleSystem::SetSmokeParticlesLightmap(const _6Way_Lightmap& lightmap)
	{
		m_smoke6WayLM = lightmap;

		for (particles::SmokeEmitter& emitter : m_smokeEmitters)
		{
			emitter.m_particleFrameTime = emitter.m_particleLifetime / static_cast<float>(lightmap.atlas.getNumSprites());
		}
	}
	void ParticleSystem::Update(float deltaTime)
	{
		uint32_t totalParticles{};
		for (particles::SmokeEmitter& emitter : m_smokeEmitters)
		{
			emitter.Update(deltaTime);
			totalParticles += emitter.NumParticlesSpawned();
		}

		m_sortedParticles.clear();
		m_sortedParticles.reserve(totalParticles);

		const math::Vector3 cameraPos{ CameraManager::GetActiveCamera().GetPosition() };

		for (uint32_t emitterIdx = 0; emitterIdx < m_smokeEmitters.size(); ++emitterIdx)
		{
			particles::SmokeEmitter& emitter = m_smokeEmitters[emitterIdx];

			for (uint32_t particleIdx = 0; particleIdx < emitter.NumParticlesSpawned(); ++particleIdx)
			{
				particles::Particle particle = emitter.m_particles[particleIdx];

				float distanceSq = math::Vector3::DistanceSquared(particle.position, cameraPos);

				m_sortedParticles.emplace_back(distanceSq, emitterIdx, particleIdx);
			}
		}

		std::sort(m_sortedParticles.begin(), m_sortedParticles.end(), [](const ParticleDistance& lval, const ParticleDistance& rval)
			{
				return lval.distanceSq > rval.distanceSq;
			});


	}

	void ParticleSystem::updateGPUSimulation(renderer::ShaderResourceView& depthBuffer, renderer::ShaderResourceView& normalsBuffer)
	{
		{
			renderer::ShaderResourceView csRes[2] = { depthBuffer, normalsBuffer };
			renderer::ShaderResourceView::BindCS(csRes, 2, TX_PART_SIM_DEPTH_SLOT);

			renderer::UnorderedAccessView csUav[3] = { m_particlesRingUAV, m_particlesRangeUAV };

			renderer::UnorderedAccessView::BindCS(csUav, 3, RW_PART_SIM_RING_BUFF_SLOT);
		}


		//update gpu particles simulation
		m_computeUpdateParticles.bind();
		m_computeUpdateParticles.dispatch(MAX_GPU_PARTICLES / 64, 1, 1);

		m_computeUpdateRangeBuffer.bind();
		m_computeUpdateRangeBuffer.dispatch(1, 1, 1);
		
		renderer::ShaderResourceView::ClearCS(TX_PART_SIM_DEPTH_SLOT);
		renderer::ShaderResourceView::ClearCS(TX_PART_SIM_NORMAL_GB_SLOT);

		renderer::UnorderedAccessView::ClearCS(RW_PART_SIM_RING_BUFF_SLOT);
		renderer::UnorderedAccessView::ClearCS(RW_PART_SIM_RANGE_BUFF_SLOT);


	}

	void ParticleSystem::AddSmokeEmitter(
		const math::Vector3& pos, 
		float spawnRate, 
		const math::Color& tint, 
		float spawnRadius, 
		Handle<math::Matrix> parentID,
		math::Vector2 xSpeedRange, 
		math::Vector2 zSpeedRange, 
		math::Vector2 initialSize,
		math::Vector2 rotationRange, 
		float increaseSizeRate,
		float ySpeed, 
		float particleLifetime,
		float maxParticles)
	{
		particles::SmokeEmitter& emitter = m_smokeEmitters.emplace_back(pos, spawnRate, tint, spawnRadius, parentID,  xSpeedRange, zSpeedRange, initialSize, rotationRange, increaseSizeRate, ySpeed, particleLifetime, maxParticles);
		emitter.m_particleFrameTime = emitter.m_particleLifetime / static_cast<float>(m_smoke6WayLM.atlas.getNumSprites());
	}

	void ParticleSystem::Draw()
	{

		renderer::D3DRenderer& renderer{ renderer::D3DRenderer::Get() };
		renderer.getDepthBufferCopy().BindPS(TX_GB_DEPTH_SLOT);
		renderer.BindRasterizerState(renderer::RSTYPES::DEFAULT);

		m_particlesRingSRV.BindVS(BUF_PARTICLE_RENDER_SLOT);
		m_particlesRangeSRV.BindVS(BUF_RANGE_RENDER_SLOT);
		m_gpuParticlesMask->BindPS(PARTICLE_MASK_SLOT);

//RENDER GPU PARTICLES
//1-Add diffuse using spheres
//2-Render particles as billboards
		renderer.BindBlendState(renderer::BSTYPES::ADDITIVE_TRANSLUCENCY);
		renderer.BindDepthStencilState(renderer::DSSTYPES::DSS_DEPTH_RO_GREATEREQ_STENCIL_RO_OPEQUAL, renderer::PBR_SHADOWS);

		m_gpuParticlesDiffusePS.Bind();
		m_gpuParticlesDiffuseVS.Bind();

#ifdef PARTICLES_LIGHTNING_SPHERES
		ModelManager::GetUnitSphereSmooth()->BindVertexBuffer();
		ModelManager::GetUnitSphereSmooth()->BindIndexBuffer();
		DX_CALL(s_devcon->DrawIndexedInstancedIndirect(m_particlesRangeBuffer.operator->(), 8 * sizeof(uint32_t)));
#elif PARTICLES_LIGHTNING_BILLBOARDS
		ModelManager::GetQuad()->BindIndexBuffer();
		DX_CALL(s_devcon->DrawIndexedInstancedIndirect(m_particlesRangeBuffer.operator->(), 3 * sizeof(uint32_t)));
#endif

		m_gpuParticlesVS.Bind();
		m_gpuParticlesPS.Bind();

		renderer.BindBlendState(renderer::BSTYPES::GENERAL_TRANSLUCENCY);
		renderer.BindDepthStencilState(renderer::DSSTYPES::DSS_DEPTH_RO_GREATER_STENCIL_DISABLED);
		ModelManager::Get().GetQuad()->BindIndexBuffer();



		DX_CALL(s_devcon->DrawIndexedInstancedIndirect(m_particlesRangeBuffer.operator->(), 3 * sizeof(uint32_t)));

		renderer::ShaderResourceView::ClearVS(BUF_PARTICLE_RENDER_SLOT);
		renderer::ShaderResourceView::ClearVS(BUF_RANGE_RENDER_SLOT);

		//Before drawing, verify if we can batch everything in a single drawcall. Max instances is: 	D3D11_SHADER_MAX_INSTANCES	( 65535 )
		const uint32_t requiredDrawCalls{ static_cast<uint32_t>(std::ceilf(static_cast<float>(m_sortedParticles.size()) / static_cast<float>(D3D11_SHADER_MAX_INSTANCES)) )};

		//RENDER CPU EMITTERS
		if (requiredDrawCalls)
		{
			auto sortedIterator = m_sortedParticles.begin();
			//bind anything you need, like textures and index buffer and shaders


			m_smokeVS.Bind();
			m_smokePS.Bind();

			m_smoke6WayLM.RLUmultipliers->BindPS(TX_PARTICLES_RLU_SLOT);
			m_smoke6WayLM.DBFmultipliers->BindPS(TX_PARTICLES_DBF_SLOT);
			m_smoke6WayLM.EMVAfactors->BindPS(TX_PARTICLES_EMVA_SLOT);


			{
				tiledAtlasUniformGPU* atlasMap = (tiledAtlasUniformGPU*)m_atlasUniform.Map();
				tiledAtlasUniformGPU upload;
				upload.width = m_smoke6WayLM.atlas.getWidth();
				upload.height = m_smoke6WayLM.atlas.getHeight();
				upload.numTiles = m_smoke6WayLM.atlas.getNumSprites();
				upload.tileWidth = m_smoke6WayLM.atlas.getSprite(0).width;
				upload.tileHeight = m_smoke6WayLM.atlas.getSprite(0).height;
				upload.maxTilesPerRow = upload.width / upload.tileWidth;
				upload.maxTilesPerCol = upload.height / upload.tileHeight;

				*atlasMap = upload;

				m_atlasUniform.Unmap();
				m_atlasUniform.BindPS(SMOKE_TILED_ATLAS_SLOT);

			}

#ifdef CAMERA_CENTER_WS
			const math::Vector3 cameraPos = CameraManager::GetActiveCamera().GetPosition();
#endif

			for (uint32_t batch = 0; batch < requiredDrawCalls; ++batch)
			{
				uint32_t particlesToDraw;
				if (batch == requiredDrawCalls - 1)//last batch draw remaining particles
				{
					particlesToDraw = static_cast<uint32_t>(m_sortedParticles.size()) - batch * D3D11_SHADER_MAX_INSTANCES;
				}
				else
				{
					particlesToDraw = D3D11_SHADER_MAX_INSTANCES;
				}

				uint32_t instancesDrawn{};
				m_batchInstanceBuffer.CreateGPUBuffer(sizeof(particles::SmokeEmitter::ParticleInstanceGPU), particlesToDraw, nullptr);

				particles::SmokeEmitter::ParticleInstanceGPU* targetPtr = static_cast<particles::SmokeEmitter::ParticleInstanceGPU*>(m_batchInstanceBuffer.Map());
				for (uint32_t i = 0; i < particlesToDraw; ++i)
				{
					//This feels like cache miss party, and looks horrible. A particle pool solution would look much better I think
					ParticleDistance& sortedParticle = *sortedIterator;
					const particles::SmokeEmitter& emitter{ m_smokeEmitters[sortedParticle.emitterIdx] };
					const particles::Particle& particle{ emitter.m_particles[sortedParticle.particleIdx] };

					particles::SmokeEmitter::ParticleInstanceGPU instance;
					instance.tint = particle.tint.ToVector4();
#ifdef CAMERA_CENTER_WS
					instance.pos = particle.position - cameraPos;
#else
					instance.pos = particle.position;
#endif
					instance.rotation = particle.rotation;
					instance.speed = particle.speed;

					float frameTimes{particle.timeAlive / emitter.m_particleFrameTime };
					float frameFraction{ frameTimes - std::floorf(frameTimes)};

					instance.timeFraction = frameFraction;
					instance.size = particle.size;
					instance.tileIdx = std::min(static_cast<uint32_t>(frameTimes), m_smoke6WayLM.atlas.getNumSprites() - 1);

					targetPtr[instancesDrawn++] = instance;
					++sortedIterator;
				}
				m_batchInstanceBuffer.Unmap();
				m_batchInstanceBuffer.Bind(1);



				DX_CALL(s_devcon->DrawIndexedInstanced(6, particlesToDraw, 0, 0, 0));
			}
		}



	}

}