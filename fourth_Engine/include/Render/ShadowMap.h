#pragma once
#include "include/Render/Texture.h"

namespace fth
{
	namespace renderer
	{
		class Camera;
	}


	struct ShadowMap
	{
	public:
		struct CubemapDepthPassBuffer
		{
			math::Matrix  VP[6];
			uint32_t      initialSlice;
			uint32_t      pad[3];
		};

		struct cbSMFrustumPlanesGPU
		{
			float  nearPlane;
			float  farPlane;
			float  nearTexelSizeWS;
			float  pad[1];
		};

		struct cbShadowMapsGPU
		{
			math::Matrix directionalVP;
			math::Matrix spotVP;
			math::Matrix pointVP[20 * 6];
			cbSMFrustumPlanesGPU  frustumPlanes[3]; //they are already padded. size is 16bytes. Wasting 4 bytes.
		};


		static constexpr float vertical_FoV_Cube = DirectX::XM_PIDIV2;

	public:
		//TODO: Add checks to recreate shadowmaps if needed
		void Init2D(float shadow_distance, float margin, float resolution, float nearPlane = 0.1f);
		void InitCubemap(float shadow_distance, float margin, float resolution,  uint16_t numCubes, float nearPlane = 0.1f);
		math::Matrix ComputeTightFrustumOrthogonal(const renderer::Camera& pov, const math::Vector3& direction, float& outNearTexelSize) const;
		math::Matrix ComputeTightFrustumPerspective(const math::Vector3& position, const math::Vector3& direction, float fov, float& outNearTexelSize) const;
		//Returns ViewProjection matrix stream for a Cubemap depth pass
		const std::vector<math::Matrix>& ComputeFrustumPerspectiveStream(const math::Vector3& position) const;

		//Binds proper viewport and depth stencil target. Clears SRV texture slot
		void BindDepthPass(uint16_t slot) const;
		//Binds texture to pipeline for shadow rendering
		void Bind(uint16_t slot) const;

		float ComputeFarPlaneOrthogonalShadowmap() const { return shadow_distance + margin; }
		float ComputeFarPlanePerspectiveShadowmap() const { return shadow_distance; }
		float ComputeNearPlaneTexelSizeFor_FoVy(float verticalFoV_radians) const;

	public:
		float shadow_distance;
		float nearPlane;
		float margin;
		float resolution;

		Texture texture;
		renderer::DepthStencilView depthTarget;
	};
}