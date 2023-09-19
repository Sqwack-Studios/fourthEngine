#pragma once
#include "include/Render/Lights.h"
#include "include/Utils/SolidVector.h"
#include "include/Render/Renderer/UniformBuffer.h"
#include "include/Render/ShadowMap.h"
#include "include/Render/Renderer/ShadingGroups/PixelShader.h"
#include "include/Render/Renderer/ShadingGroups/VertexShader.h"


namespace fth
{
	class LightSystem
	{
	public:
		using LightID = uint32_t;

		static LightSystem& Get()
		{
			static LightSystem instance;
			return instance;
		}


		void Init();
		void Shutdown();
		void Update();
		LightSystem::LightID AddDirectionalSphereLight(math::Vector3 luminance, math::Vector3 direction, float solidAngle, Handle<math::Matrix> parentID);
		LightSystem::LightID AddPointSphereLight(math::Vector3 radiance, float radius, math::Vector3 position, Handle<math::Matrix> parentID);
		LightSystem::LightID AddSpotSphereLight(
			math::Vector3 radiance,
			float radius, 
			math::Vector3 position, 
			math::Vector3 orientation, 
			float radInnerAngle, 
			float radOuterAngle,
			Handle<math::Matrix> parentID,
			const std::shared_ptr<Texture>& mask);
																					  
		lights::DirectionalSphere& QueryDirectionalLight(uint32_t ID) { return m_directionalLights[ID]; }
		lights::PointSphere& QueryPointLight(uint32_t ID) { return m_pointLights[ID]; }
		lights::SpotSphere& QuerySpotLight(uint32_t ID) { return m_spotLights[ID]; }
																					  
		lights::DirectionalSphere& QueryDirectionalLightByIndex(uint32_t index) { return m_directionalLights.at(index); }
		lights::PointSphere& QueryPointLightByIndex(uint32_t index) { return m_pointLights.at(index); }
		lights::SpotSphere& QuerySpotLightByIndex(uint32_t index) { return m_spotLights.at(index); }

		uint32_t QueryDirectionalLightIndex(uint32_t ID) { return m_directionalLights.getIndexByID(ID); }
		uint32_t QueryPointLightIndex(uint32_t ID) { return m_pointLights.getIndexByID(ID); }
		uint32_t QuerySpotLightIndex(uint32_t ID) { return m_spotLights.getIndexByID(ID); }

		uint32_t GetNumPointLights() const         { return static_cast<uint32_t>(m_pointLights.size()); }
		uint32_t GetNumSpotLights() const          { return static_cast<uint32_t>(m_spotLights.size()); }
		uint32_t GetNumDirectionalLights() const   { return static_cast<uint32_t>(m_directionalLights.size()); }

		ShadowMap& GetDirectionalShadowmap() { return m_directionalShadowmap; }
		ShadowMap& GetSpotlightShadowmap() { return m_spotShadowmap; }
		ShadowMap& GetPointShadowmap() { return m_pointShadowmap; }

	private:
		~LightSystem();
		LightSystem();

		void InitLights();
		void InitShadowMaps();
	private:
		struct cbNumberLightsGPU
		{
			//uint32_t  numDirectional;
			uint32_t  numPoint;
			//uint32_t  numSpot;
			uint32_t  pad[3];
		};



		SolidVector<lights::DirectionalSphere>          m_directionalLights;
		SolidVector<lights::PointSphere>                m_pointLights;
		SolidVector<lights::SpotSphere>                 m_spotLights;


		ShadowMap                                       m_directionalShadowmap;
		ShadowMap                                       m_spotShadowmap;
		ShadowMap                                       m_pointShadowmap;

		renderer::UniformBuffer                         m_gpuLightData;
									                    
	};
}