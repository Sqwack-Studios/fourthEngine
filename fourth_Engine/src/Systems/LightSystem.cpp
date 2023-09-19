#include "pch.h"
#pragma once
#include "include/Systems/LightSystem.h"
#include "include/Render/Renderer/D3DRenderer.h"
#include "include/Managers/CameraManager.h"
#include "include/Systems/TransformSystem.h"
#include "Shaders/RegisterSlots.hlsli"
#include "include/Render/Texture.h"
#include "include/Systems/MeshSystem.h"
#include "include/Specifications.h"

namespace fth
{
	LightSystem::LightSystem() {}
	LightSystem::~LightSystem() {}

	void LightSystem::Init()
	{
		InitLights();
		InitShadowMaps();
	}

	void LightSystem::InitLights()
	{
		m_directionalLights.reserve(Config::MAX_DIRECITONAL_LIGHTS);
		m_pointLights.reserve(Config::MAX_POINT_LIGHTS);
		m_spotLights.reserve(Config::MAX_SPOT_LIGHTS);


		const uint32_t lightsCount[3] = { 0, 0, 0 };

		uint32_t lightsCountByteSize{ sizeof(LightSystem::cbNumberLightsGPU) };
		uint32_t directionalByteSize{ Config::MAX_DIRECITONAL_LIGHTS * sizeof(lights::cbDirectSphereGPU) };
		uint32_t pointByteSize{ Config::MAX_POINT_LIGHTS * sizeof(lights::cbPointSphereGPU) };
		uint32_t spotByteSize{ Config::MAX_SPOT_LIGHTS * sizeof(lights::cbSpotSphereGPU) };

		uint32_t byteSize{
		   lightsCountByteSize +
		   directionalByteSize +
			pointByteSize +
		   spotByteSize };

		m_gpuLightData.CreateGPUBuffer(byteSize, 1, nullptr);

		//Copy initial data to the buffer, which is uninitialized except lights count
		uint8_t* dst = (uint8_t*)m_gpuLightData.Map();
		memcpy(dst, &lightsCount, lightsCountByteSize);
		dst += lightsCountByteSize;

		{
			static lights::cbDirectSphereGPU dummy[Config::MAX_DIRECITONAL_LIGHTS];
			memcpy(dst, dummy, Config::MAX_DIRECITONAL_LIGHTS * sizeof(lights::cbDirectSphereGPU));
			dst += Config::MAX_DIRECITONAL_LIGHTS * sizeof(lights::cbDirectSphereGPU);
		}
		{
			static lights::cbPointSphereGPU dummy[Config::MAX_POINT_LIGHTS];
			memcpy(dst, dummy, Config::MAX_POINT_LIGHTS * sizeof(lights::cbPointSphereGPU));
			dst += Config::MAX_POINT_LIGHTS * sizeof(lights::cbPointSphereGPU);
		}
		{
			static lights::cbSpotSphereGPU dummy[Config::MAX_SPOT_LIGHTS];
			memcpy(dst, dummy, Config::MAX_SPOT_LIGHTS * sizeof(lights::cbSpotSphereGPU));
			dst += sizeof(lights::cbSpotSphereGPU);
		}

		m_gpuLightData.Unmap();
	}

	void LightSystem::InitShadowMaps()
	{
		m_directionalShadowmap.Init2D(50.0f, 5.0f, 1024.0f);
		m_spotShadowmap.Init2D(50.0f, 5.0f, 1024.0f);

		//If max point lights is 20, and resolution is 4096, that is:
		//4096x4096 = 16 777 216 px, at 32 bit format 
		// 536 870 912 bits = 67 108 864 bytes/texture
		//we have 20 * 6 textures = 120, 8 053 063 680 bytes = 8.05 GB!!!!

		m_pointShadowmap.InitCubemap(50.0f, 5.0f, 1024.0f, Config::MAX_POINT_LIGHTS);
	}

	void LightSystem::Shutdown()
	{

	}

	void LightSystem::Update()
	{
		const cbNumberLightsGPU lightsCount = { static_cast<uint32_t>(m_pointLights.size())};


		//Total size of the GPU buffer: num of each light type + each light struct * num of light type

		uint8_t* dst = static_cast<uint8_t * >(m_gpuLightData.Map());
		memcpy(dst, &lightsCount, sizeof(cbNumberLightsGPU));
		dst += sizeof(cbNumberLightsGPU);


		//Update light data, first directional, point and finally spot

#ifdef ILLUMINATION_VIEW_SPACE
		const math::Matrix cameraView{ CameraManager::GetActiveCamera().GetView() };
#elif  CAMERA_CENTER_WS
		const math::Matrix invCameraView{ CameraManager::GetActiveCamera().GetInverseView() };
#endif
		{
			if (m_directionalLights.size() > 0)
			{
				lights::DirectionalSphere& light{ m_directionalLights.at(0) };
				const math::Matrix parentTransform = light.parentID.isValid() ? TransformSystem::Get().QueryTransformMatrix(light.parentID) : math::Matrix::Identity;

				math::Vector3 direction {math::Vector3::TransformRotate(light.direction, parentTransform)};

#ifdef ILLUMINATION_VIEW_SPACE
				direction = math::Vector3::TransformRotate(direction, cameraView);
#endif

				const lights::cbDirectSphereGPU& gpuDirectional{ light.luminance, light.solidAngle, direction };
				lights::cbDirectSphereGPU* dstDirect = reinterpret_cast<lights::cbDirectSphereGPU*>(dst);

				*dstDirect = gpuDirectional;
			}

			dst += sizeof(lights::cbDirectSphereGPU);
		}


		if(lightsCount.numPoint == 0)
			dst += Config::MAX_POINT_LIGHTS * sizeof(lights::cbPointSphereGPU);
		else
		{
			for (uint32_t lightIdx = 0; lightIdx < lightsCount.numPoint; ++lightIdx)
			{
				lights::PointSphere& light{ m_pointLights.at(lightIdx) };
				math::Matrix parentTransform = light.parentID.isValid() ? TransformSystem::Get().QueryTransformMatrix(light.parentID) : math::Matrix::Identity;

				math::Vector3 position { math::Vector3::Transform(light.position, parentTransform) };

#ifdef ILLUMINATION_VIEW_SPACE
	
				position = math::Vector3::Transform(position, cameraView);

#elif CAMERA_CENTER_WS
				position -= invCameraView.Translation();
#endif

				const lights::cbPointSphereGPU& gpuPoint{ light.radiance, light.radius, position};
				lights::cbPointSphereGPU* dstPoint = reinterpret_cast<lights::cbPointSphereGPU*>(dst);

				*dstPoint = gpuPoint;
				dst += sizeof(lights::cbPointSphereGPU);

			}
			dst += (Config::MAX_POINT_LIGHTS - lightsCount.numPoint) * sizeof(lights::cbPointSphereGPU);

		}


		{
			if (m_spotLights.size() > 0)
			{
				lights::SpotSphere& light{ m_spotLights.at(0) };

				if (light.textureMask)
					light.textureMask->BindPS(TX_LIGHTMASK_SLOT);

				const math::Matrix parentTransform = light.parentID.isValid() ? TransformSystem::Get().QueryTransformMatrix(light.parentID) : math::Matrix::Identity;

				math::Vector3 position { math::Vector3::Transform(light.position, parentTransform) };
				math::Vector3 direction { math::Vector3::TransformRotate(light.direction, parentTransform) };

				math::Matrix lightView{};
#ifdef ILLUMINATION_VIEW_SPACE

				position = math::Vector3::Transform(position, cameraView);
				direction = math::Vector3::TransformRotate(direction, cameraView);

#elif CAMERA_CENTER_WS

				position -=  invCameraView.Translation();
#endif

				lightView =  math::Matrix::CreateLookAtLH(position, position + direction, math::Vector3::Up);
				

				const math::Matrix viewProjection = lightView * light.projection;

				const lights::cbSpotSphereGPU& gpuSpot{
					light.radiance,
					light.radius,
					position,
					light.angleScale,
					direction,
					light.angleOffset,
					viewProjection
				};
				lights::cbSpotSphereGPU* dstSpot = reinterpret_cast<lights::cbSpotSphereGPU*>(dst);

				*dstSpot = gpuSpot;
			}

			dst += sizeof(lights::cbSpotSphereGPU);

		}

		m_gpuLightData.Unmap();
		m_gpuLightData.BindPS(LIGHTS_SLOT);
		m_gpuLightData.BindVS(LIGHTS_SLOT);

	}

	LightSystem::LightID LightSystem::AddDirectionalSphereLight(math::Vector3 luminance, math::Vector3 direction, float solidAngle, Handle<math::Matrix> parentID)
	{
		if (m_directionalLights.size() == Config::MAX_DIRECITONAL_LIGHTS)
		{
			LOG_ENGINE_WARN("LightSystem", "Maximum amount of directional lights reached. Returned las light ID");
			return Config::MAX_DIRECITONAL_LIGHTS - 1;
		}
	
		lights::DirectionalSphere light(luminance, direction, solidAngle, parentID);
		

		return m_directionalLights.insert(light);

	}


	LightSystem::LightID LightSystem::AddPointSphereLight(math::Vector3 radiance, float radius, math::Vector3 position, Handle<math::Matrix> parentID)
	{
		if (m_pointLights.size() == Config::MAX_POINT_LIGHTS)
		{
			LOG_ENGINE_WARN("LightSystem", "Maximum amount of point lights reached. Returned las light ID");
			return Config::MAX_POINT_LIGHTS - 1;
		}

		lights::PointSphere light (radiance, radius, position, parentID);

		return m_pointLights.insert(light);
	}

	LightSystem::LightID LightSystem::AddSpotSphereLight(
		math::Vector3 radiance,
		float radius,
		math::Vector3 position,
		math::Vector3 orientation,
		float radInnerAngle,
		float radOuterAngle,
		Handle<math::Matrix> parentID,
		const std::shared_ptr<Texture>& mask)
	{
		if (m_spotLights.size() == Config::MAX_SPOT_LIGHTS)
		{
			LOG_ENGINE_WARN("LightSystem", "Maximum amount of spot lights reached. Returned las light ID");
			return Config::MAX_POINT_LIGHTS - 1;
		}

		lights::SpotSphere light(radiance, radius, position, parentID, orientation, radInnerAngle, radOuterAngle, mask);

		return m_spotLights.insert(light);
	}



}