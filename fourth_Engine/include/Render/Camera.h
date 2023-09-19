#pragma once
#include "include/Systems/TransformSystem.h"


namespace fth
{
	//                                    Casts to
	//               Focal Lenght      |  ----->
	//        |				           |
	//        xPosition----------------|-------------------------->Forward Vector(unit)
	//	      |			               |
	//                                 |  ----->
	//                              View plane
	//
	// 
	//           View Plane Center Point = xPos + FL * Forward 
	//          Calculate dx and dy based on view plane size and pixel arrays, calculate world position for "each pixel", cast ray
	//

	struct Ray;
	struct MousePos;
	namespace renderer
	{
		//FOV field is specified in degrees
		struct ViewportSettings
		{
			float fov;
			float nearPlane;
			float farPlane;
			float aspectRatio;
		};

		class Camera
		{
		public:
			Camera();
			Camera(const math::Vector3& position, const math::Vector3& forwardVector);
			Camera(const math::Vector3& position, const math::Vector3& forwardVector, const ViewportSettings& settings);

			math::Vector3 GetPosition() const { return GetInverseView().Translation(); }
			math::Vector3 GetForward() const  { return GetInverseView().Forward(); }
			math::Vector3 GetRight() const    { return GetInverseView().Right(); }
			math::Vector3 GetUp() const       { return GetInverseView().Up(); }
			const math::Quaternion& GetRotation() const  { return m_Rotation; }

			void ToggleFixedBottom() { m_fixedBottom = !m_fixedBottom; }

			ViewportSettings GetViewportSettings() const { return { m_fov, m_nearPlane, m_farPlane, m_aspectRatio }; }
			void UpdateViewPortSettings(const ViewportSettings& settings);

			const math::Frustum& getFrustum() const { return m_frustumCorners; }

		public:

			inline const float& GetNearPlane() const { return m_nearPlane; }
			void SetNearPlane(float nearPlane) { m_nearPlane = nearPlane, RecalculateProjectionMatrix(); }

			inline const float& GetFarPlane() const { return m_farPlane; }
			inline void SetFarPlane(float farPlane) { m_farPlane = farPlane, RecalculateProjectionMatrix(); }


			inline const float& GetFoV() const { return m_fov; }
			inline void SetFoV(float fov) { m_fov = fov, RecalculateProjectionMatrix(); }

			inline void SetAspectRatio(float aspectRatio) { m_aspectRatio = aspectRatio, RecalculateProjectionMatrix(); }
			inline const float& GetAspectRatio() const { return m_aspectRatio; }


		public:

			void SetWorldPosition(const math::Vector3& targetPos);
			void AddWorldPosition(const math::Vector3& deltaPos);
			void AddRelativePos(const math::Vector3& deltaPos);

			void TranslateFixedBottom(const math::Vector3& deltaPos);
			void RotateFixedBottom(float pitch, float yaw);
			void SetWorldLookAtDirection(const math::Vector3& forward, const math::Vector3& up);
			void SetWorldAngles(float roll, float pitch, float yaw);
			void AddWorldAngles(float deltaRoll, float deltaPitch, float deltaYaw);
			void AddRelativeAngles(float deltaRoll, float deltaPitch, const float deltaYaw);

		public:
			const math::Matrix& GetView() const                   { return TransformSystem::Get().QueryTransformMatrix(m_idView); }
			const math::Matrix& GetInverseView() const            { return TransformSystem::Get().QueryTransformMatrix(m_idInvView); }
			const math::Matrix& GetProjection() const             { return TransformSystem::Get().QueryTransformMatrix(m_idProjection); }
			const math::Matrix& GetInverseProjection() const      { return TransformSystem::Get().QueryTransformMatrix(m_idInvProjection); }
			const math::Matrix& GetViewProjection() const         { return TransformSystem::Get().QueryTransformMatrix(m_idViewProj); }
			const math::Matrix& GetInverseViewProjection() const  { return TransformSystem::Get().QueryTransformMatrix(m_idInvViewProj); }


			math::Matrix& GetView()                               { return TransformSystem::Get().QueryTransformMatrix(m_idView); }
			math::Matrix& GetInverseView()                        { return TransformSystem::Get().QueryTransformMatrix(m_idInvView); }
			math::Matrix& GetProjection()                         { return TransformSystem::Get().QueryTransformMatrix(m_idProjection); }
			math::Matrix& GetInverseProjection()                  { return TransformSystem::Get().QueryTransformMatrix(m_idInvProjection); }
			math::Matrix& GetViewProjection()                     { return TransformSystem::Get().QueryTransformMatrix(m_idViewProj); }
			math::Matrix& GetInverseViewProjection()              { return TransformSystem::Get().QueryTransformMatrix(m_idInvViewProj); }

			const Handle<math::Matrix>& GetIDView() const                  { return m_idView; }
			const Handle<math::Matrix>& GetIDInverseView() const           { return m_idInvView; }
			const Handle<math::Matrix>& GetIDProjection() const            { return m_idProjection; }
			const Handle<math::Matrix>& GetIDInverseProjection() const     { return m_idInvProjection; }
			const Handle<math::Matrix>& GetIDViewProjection() const        { return m_idViewProj; }
			const Handle<math::Matrix>& GetIDInverseViewProjection() const { return m_idInvViewProj; }


			void BuildRayFromScreenPixel(uint16_t targetX, uint16_t targetY, uint16_t bufferWidth, uint16_t bufferHeight, Ray& outRay) const;

			void Project(const math::Vector4& target, math::Vector4& outVector);
			void Unproject(const math::Vector4& target, math::Vector4& outVector);

			void CameraToWorld(const math::Vector4& target, math::Vector4& outVector);
			math::Vector3 WorldToCamera(const math::Vector3& target, bool translation);

			math::Vector3 ClipToWorld(const math::Vector3& target, bool translation);
			void WorldToClip(const math::Vector4& target, math::Vector4& outVector);

			void UpdateMatrices();


		private:

			//On resize
			void RecalculateProjectionMatrix();

			void RecalculateViewportSizeInWorldSpace();

			void RecalculateRotations();

			void NormalizeRotation();



		private:
			friend class D3DRenderer;
			float                            m_nearPlane;
			float                            m_farPlane;
			float                            m_fov;
			float                            m_aspectRatio;

			math::Frustum                    m_frustumCorners;

			math::Quaternion                 m_Rotation;

			//World -> Camera
			Handle<math::Matrix>                      m_idView;
			Handle<math::Matrix>                      m_idInvView;

			//Camera -> Clip
			Handle<math::Matrix>                      m_idProjection;
			Handle<math::Matrix>                      m_idInvProjection;

			//World -> Clip: VP matrix
			Handle<math::Matrix>                      m_idViewProj;
			Handle<math::Matrix>                      m_idInvViewProj;


			//size of the viewport in world space. pixel coordinates are given to the camera
			float                            m_ViewPortHeight; 	  //near plane
			float                            m_ViewPortWidth;	  //near plane

			float                            m_farPlaneHeight;
			float                            m_farPlaneWidth;

			uint16_t                         m_normalizeRotIndex;

			bool                             m_updatedBasis;
			bool                             m_updatedVP;
			bool                             m_fixedBottom;





		};
	}



	
}