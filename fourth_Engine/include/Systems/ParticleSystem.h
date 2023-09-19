#pragma once
#include "include/Render/Particles/SmokeEmitter.h"
#include "include/Render/Renderer/InstanceBuffer.h"
#include "include/Render/Renderer/UniformBuffer.h"
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"
#include "include/Render/Renderer/ComputeShader.h"
#include "include/Render/Renderer/D3DView.h"
#include "include/Render/Renderer/Buffer.h"
#include "include/Render/Atlas.h"
//We could use the same data oriented pattern from Shading groups. Group emitters by combination of textures, and render them based on their textures. 
//Let's keep it simple for now, altought it shouldnt be hard to implement, giving us lot of flexibility

namespace fth
{
	class Texture;

	struct _6Way_Lightmap
	{
		std::shared_ptr<Texture> RLUmultipliers;//right, left, top
		std::shared_ptr<Texture> DBFmultipliers;//bottom, back, front
		std::shared_ptr<Texture> EMVAfactors;//emission, motion, alpha

		Atlas                    atlas;
	};

	class ParticleSystem
	{
	public:
		struct ringGPUParticle;

		static constexpr uint32_t MAX_GPU_PARTICLES = 1024;

		ParticleSystem(const ParticleSystem&) = delete;
		ParticleSystem(ParticleSystem&&) = delete;
		ParticleSystem& operator=(const ParticleSystem&) = delete;
		ParticleSystem& operator=(ParticleSystem&&) = delete;
		~ParticleSystem();

		static ParticleSystem& Get()
		{
			static ParticleSystem singleton;
			return singleton;
		}
		void Init();
		void Shutdown();
		//Updates emitter's particle positions and sorts them by distance to main camera
		void Update(float deltaTime);
		void updateGPUSimulation(renderer::ShaderResourceView& depthBuffer, renderer::ShaderResourceView& normalsBuffer);
		void SetSmokeParticlesLightmap(const _6Way_Lightmap& lightmap);
		void AddSmokeEmitter(const math::Vector3& pos,
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
			float maxParticles);

		//Draws particles of all emitters into the screen. Particles of the same type will be batched together.
		const renderer::UnorderedAccessView& getParticlesRingUAV() const { return m_particlesRingUAV; }
		const renderer::UnorderedAccessView& getParticlesRangeUAV() const { return m_particlesRangeUAV; }
		void setGPUParticlesMask(const std::shared_ptr<Texture>& mask) { m_gpuParticlesMask = mask; };

		void Draw();
	private:
		ParticleSystem();
		std::vector<particles::SmokeEmitter>  m_smokeEmitters;
		_6Way_Lightmap                        m_smoke6WayLM;

		struct ParticleDistance
		{
			ParticleDistance(float distanceSq, uint32_t emitterIdx, uint32_t particleIdx) : 
				distanceSq(distanceSq), emitterIdx(emitterIdx), particleIdx(particleIdx) {}

			bool operator>(const ParticleDistance& rval) { return distanceSq > rval.distanceSq; }
			bool operator<(const ParticleDistance& rval) { return distanceSq > rval.distanceSq; }

			float distanceSq;//distance squared!!!
			uint32_t emitterIdx;
			uint32_t particleIdx;
		};


		struct tiledAtlasUniformGPU
		{
			uint32_t    width;
			uint32_t    height;
			uint32_t    numTiles;
			uint32_t    tileWidth;

			uint32_t    tileHeight;
			uint32_t    maxTilesPerRow;
			uint32_t    maxTilesPerCol;
			float       pad[1];
		};

		std::vector<ParticleDistance>         m_sortedParticles;
		renderer::InstanceBuffer              m_batchInstanceBuffer;
		renderer::UniformBuffer               m_atlasUniform;
		renderer::VertexShader                m_smokeVS;
		renderer::PixelShader                 m_smokePS;


		//GPU particle system
	public:
		struct ringGPUParticle
		{
			math::Vector4  color;
			//
			math::Vector3  position;
			float          rotation;
			//
			math::Vector3  speed;
			float          timeLeft;
			//
			math::Vector2  size;
			float          pad[2]; //allign to 16bytes
		};

		struct drawArgs
		{
			uint32_t       indexCountPerInstance;
			//
			uint32_t       instanceCount;
			uint32_t       startIndexLocation;
			uint32_t       baseVertexLocation;
			uint32_t       startInstanceLocation;
		};
		struct ringBufferRange
		{
			uint32_t       number;
			uint32_t       offset;
			uint32_t       expired;
			drawArgs       particleArgs;
#ifdef PARTICLES_LIGHTNING_SPHERES
			drawArgs       sphereDrawArgs;
#endif
		};

	private:
		renderer::ComputeShader                                             m_computeUpdateParticles;
		renderer::ComputeShader                                             m_computeUpdateRangeBuffer;

		renderer::PixelShader                                               m_gpuParticlesPS;
		renderer::VertexShader                                              m_gpuParticlesVS;
		renderer::PixelShader                                               m_gpuParticlesDiffusePS;
		renderer::VertexShader                                              m_gpuParticlesDiffuseVS;

		renderer::ByteAddressBuffer                                         m_particlesRangeBuffer;
		renderer::UnorderedAccessView                                       m_particlesRangeUAV;
		renderer::ShaderResourceView                                        m_particlesRangeSRV;

		renderer::StructuredBuffer<ringGPUParticle>                         m_particlesRingBuffer;
		renderer::UnorderedAccessView                                       m_particlesRingUAV;
		renderer::ShaderResourceView                                        m_particlesRingSRV;
		std::shared_ptr<Texture>                                            m_gpuParticlesMask;
	};
}