#include "pch.h"

#pragma once
#include "include/Render/Camera.h"
#include "include/Math/Ray.h"
#include "include/Math/MathUtils.h"

namespace fth
{
	namespace renderer
	{
		Camera::Camera() :
			Camera(math::Vector3::Zero, math::Vector3::Zero) 
		{

		}

		Camera::Camera(const math::Vector3& position, const math::Vector3& forwardVector) :
			Camera(position, forwardVector, ViewportSettings{ 60.f, 0.1f, 10000.f, 16.0f / 9.0f })
		{

		}


		Camera::Camera(const math::Vector3& position, const math::Vector3& forwardVector, const ViewportSettings& settings) :
			m_normalizeRotIndex(0),
			m_fixedBottom(true)
		{

			math::Vector3 fwd = forwardVector;
			fwd.Normalize();
			//Setup view matrices
			m_idProjection = TransformSystem::Get().AddTransform(math::Matrix::Identity);
			m_idInvProjection = TransformSystem::Get().AddTransform(math::Matrix::Identity);
			m_idView = TransformSystem::Get().AddTransform(math::Matrix::Identity);
			m_idInvView = TransformSystem::Get().AddTransform(math::Matrix::Identity);
			m_idViewProj = TransformSystem::Get().AddTransform(math::Matrix::Identity);
			m_idInvViewProj = TransformSystem::Get().AddTransform(math::Matrix::Identity);

			SetWorldPosition(position);
			SetWorldLookAtDirection(fwd, math::Vector3::Up);

			//Setup viewport and projection matrices
			UpdateViewPortSettings(settings);

			UpdateMatrices();
		}

		void Camera::UpdateViewPortSettings(const ViewportSettings& settings)
		{
			m_aspectRatio = settings.aspectRatio;
			m_fov = settings.fov;
			m_farPlane = settings.farPlane;
			m_nearPlane = settings.nearPlane;

			//Extract size of the viewport

			float sinHalfAngle;
			float cosHalfAngle;
			DirectX::XMScalarSinCos(&sinHalfAngle, &cosHalfAngle, DirectX::XMConvertToRadians(0.5f * GetViewportSettings().fov));
			float tanHalfAngle{ sinHalfAngle / cosHalfAngle };

			m_ViewPortHeight = 2.0f * tanHalfAngle * m_nearPlane;
			m_ViewPortWidth = 2.0f * tanHalfAngle * m_nearPlane * m_aspectRatio;

			RecalculateProjectionMatrix();

		}








		void Camera::RecalculateProjectionMatrix()
		{
			m_updatedVP = false;

			math::Matrix& proj{ GetProjection() };
			math::Matrix& invProj{ GetInverseProjection() };
			proj = math::Matrix::CreatePerspectiveFieldOfViewReversedLH(DirectX::XMConvertToRadians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
			proj.Invert(invProj);

			//Recalculate viewport size in world space

			UpdateMatrices();

			RecalculateViewportSizeInWorldSpace();

		}



		void Camera::RecalculateViewportSizeInWorldSpace()
		{
			//Project frustum corners from clip space to world space
			constexpr math::Vector3 TLinClip{ -1.0f, 1.0f, 1.0f};
			constexpr math::Vector3 BLinClip{ -1.0f, -1.0f, 1.0f};
			constexpr math::Vector3 BRinClip{ 1.0f, -1.0f, 1.0f};
			
			constexpr math::Vector3 backTL{ -1.0f, 1.0f, 0.0f};

			
			m_frustumCorners.ComputeFrustumCorners(GetInverseViewProjection());
			m_frustumCorners.ComputeFrustumCornersDirections(GetInverseView(), GetInverseProjection());

		}



		void Camera::RecalculateRotations()
		{
			if (m_updatedBasis)
				return;

			m_updatedBasis = true;

			math::Matrix newRotationMatrix{ math::Matrix::CreateFromQuaternion(m_Rotation) };

			math::Matrix& invView{ GetInverseView() };
			math::Matrix& view{ GetView() };
			//Copy the rotation matrix to the view matrix directly
			math::SetMatrix3IntoMatrix4(newRotationMatrix, invView);
			//Copy the transposed rotation matrix to the view matrix directly
			math::SetMatrix3TransposedIntoMatrix4(newRotationMatrix, view);

			//unscaled
			math::Vector3 positionRow{ invView.Translation() };

			view.Translation(-math::Vector3(positionRow.Dot(GetRight()), positionRow.Dot(GetUp()), positionRow.Dot(GetForward())));




		}

		void Camera::NormalizeRotation()
		{
			++m_normalizeRotIndex;
			if (m_normalizeRotIndex > 100)
			{
				m_Rotation.Normalize();
				m_normalizeRotIndex = 0;
			}
		}

		void Camera::UpdateMatrices()
		{
			if (m_updatedVP)
				return;

			m_updatedVP = true;

			RecalculateRotations();

			math::Matrix& viewProj{ GetViewProjection() };
			math::Matrix& invViewProj{ GetInverseViewProjection() };

			viewProj = GetView() * GetProjection();
			invViewProj = GetInverseProjection() * GetInverseView();
			RecalculateViewportSizeInWorldSpace();
		}



		void Camera::SetWorldPosition(const math::Vector3& targetPos)
		{
			m_updatedVP = false;

			GetInverseView().Translation(targetPos);
		}

		void Camera::AddWorldPosition(const math::Vector3& deltaPos)
		{
			m_updatedVP = false;
			math::Matrix& invView{ GetInverseView() };

			math::Vector3 newPos{ invView.Translation() + deltaPos };

			invView.Translation(newPos);

		}

		void Camera::AddRelativePos(const math::Vector3& deltaPos)
		{
			//We have to make sure that the rotation matrix, the basis vectors, are updated to add the position
			math::Matrix& invView{ GetInverseView() };

			//if (m_fixedBottom)
			//{
			//	TranslateFixedBottom(deltaPos);
			//	return;
			//}
			RecalculateRotations();

			m_updatedVP = false;
			math::Vector3 newPos{ (deltaPos.x * GetRight() + deltaPos.y * GetUp() + deltaPos.z * GetForward()) + GetPosition() };

			invView.Translation(newPos);
		}

		void Camera::SetWorldLookAtDirection(const math::Vector3& forward, const math::Vector3& up)
		{
			m_updatedBasis = false;
			m_updatedVP = false;

			math::Quaternion::LookRotation(forward, up, m_Rotation);
			m_Rotation.Normalize();

		}

		void Camera::SetWorldAngles(float roll, float pitch, float yaw)
		{
			m_updatedBasis = false;
			m_updatedVP = false;

			//Calculations are actually made: roll->pitch->yaw
			m_Rotation = math::Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);
			NormalizeRotation();
		}

		void Camera::AddWorldAngles(float deltaRoll, float deltaPitch, float deltaYaw)
		{
			m_updatedBasis = false;
			m_updatedVP = false;

			m_Rotation *= math::Quaternion::CreateFromYawPitchRoll(deltaYaw, deltaPitch, deltaRoll);
			NormalizeRotation();
		}

		void Camera::AddRelativeAngles(float deltaRoll, float deltaPitch, float deltaYaw)
		{
			if (m_fixedBottom)
			{
				RotateFixedBottom(deltaPitch, deltaYaw);
				return;
			}

			m_updatedBasis = false;
			m_updatedVP = false;

			m_Rotation *= math::Quaternion::CreateFromAxisAngle(GetForward(), deltaRoll) * math::Quaternion::CreateFromAxisAngle(GetRight(), deltaPitch) * math::Quaternion::CreateFromAxisAngle(GetUp(), deltaYaw);
			NormalizeRotation();
		}

		void Camera::RotateFixedBottom(float pitch, float yaw)
		{
			m_updatedBasis = false;
			m_updatedVP = false;

			m_Rotation *= math::Quaternion::CreateFromAxisAngle(GetRight(), pitch) * math::Quaternion::CreateFromAxisAngle({ 0.0f, 1.0f, 0.0f }, yaw);
			NormalizeRotation();
		}

		void Camera::TranslateFixedBottom(const math::Vector3& deltaPos)
		{
			RecalculateRotations();

			m_updatedVP = false;
			math::Vector3 newPos{ (deltaPos.x * GetRight() + deltaPos.y * GetUp() + deltaPos.z * GetForward()) + GetPosition()};
			GetInverseView().Translation(newPos);
		}

		void Camera::Project(const math::Vector4& target, math::Vector4& outVector)
		{
			outVector = { XMVector4Transform(target, GetProjection()) };
			outVector /= outVector.w;
		}

		void Camera::Unproject(const math::Vector4& target, math::Vector4& outVector)
		{
			outVector = { XMVector4Transform(target, GetInverseProjection()) };
			outVector /= outVector.w;
		}

		void Camera::CameraToWorld(const math::Vector4& target, math::Vector4& outVector)
		{
			outVector = { XMVector4Transform(target, GetInverseView()) };
		}

		math::Vector3 Camera::WorldToCamera(const math::Vector3& target, bool translation)
		{
			math::Vector4 vector = target;
			vector.w = translation;
			return { XMVector4Transform(vector, GetView()) };
		}

		math::Vector3 Camera::ClipToWorld(const math::Vector3& target, bool translation)
		{
			math::Vector4 vector = target;
			vector.w = translation;

			vector = XMVector4Transform(vector, GetInverseViewProjection());

			return math::Vector3(vector /= vector.w);
		}

		void Camera::WorldToClip(const math::Vector4& target, math::Vector4& outVector)
		{
			outVector = XMVector4Transform(target, GetViewProjection());
			outVector /= outVector.w;
		}

		void Camera::BuildRayFromScreenPixel(uint16_t targetX, uint16_t targetY, uint16_t bufferWidth, uint16_t bufferHeight, Ray& outRay) const
		{
			float half_viewPortHeight{ m_ViewPortHeight / 2.0f };
			float half_viewPlaneWidth{ m_ViewPortWidth / 2.0f };
			float dx = m_ViewPortWidth / static_cast<float>(bufferWidth);
			float dy = m_ViewPortHeight / static_cast<float>(bufferHeight);


			float pX = dx * (targetX + 0.5f);
			float pY = dy * (bufferHeight - targetY + 0.5f);



//#ifdef CAMERA_CENTER_WS
//			outRay.position = math::Vector3::Zero;
//			outRay.direction = (m_BRDirectionInWorld * cojonesX)  +   (m_TLDirectionInWorld * cojonesY);
//#else
//			outRay.position = GetPosition();
//			math::Vector4 rayTargetViewPlanePos = pX * GetRight() + pY * GetUp() + m_BLposInWorld;
//			outRay.direction = rayTargetViewPlanePos - GetPosition();
//#endif

			outRay.position = GetPosition();
			math::Vector4 rayTargetViewPlanePos = pX * GetRight() + pY * GetUp() + m_frustumCorners.getFront_BL();
			outRay.direction = rayTargetViewPlanePos - GetPosition();
			outRay.direction.Normalize();





		}
	}
}