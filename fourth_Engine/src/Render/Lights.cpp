#include "pch.h"
#pragma once
#include "include/Render/Lights.h"

namespace fth::lights
{

	//--------------Physical lights-------------------
	 
	//--------------   Directional -------------------
	DirectionalSphere::DirectionalSphere(math::Vector3 luminance, math::Vector3 inDirection, float solidAngle, Handle<math::Matrix> parentID):
		luminance(luminance),
		solidAngle(solidAngle),
		parentID(parentID)
	{
		inDirection.Normalize(direction);
	}


	//-------------- Point Sphere ---------------------
	PointSphere::PointSphere(math::Vector3 radiance, float radius, math::Vector3 position, Handle<math::Matrix> parentID):
		radiance(radiance),
		radius(radius),
		position(position),
		parentID(parentID)
	{
	}

	void PointSphere::UpdateRadiance(math::Vector3 luminousFlux, float inRadius)
	{
		//incoming vector3 is luminous flux W
		float radius2 = inRadius * inRadius;
		float divPI2 = DirectX::XM_1DIVPI * DirectX::XM_1DIVPI;

		radiance = luminousFlux / (4.0f * divPI2 * radius2);
		radius = inRadius;
	}



	//--------------- Spot Sphere ------------------
	SpotSphere::SpotSphere(
		math::Vector3 inRadiance,
		float inRadius,
		math::Vector3 inPosition,
		Handle<math::Matrix> parentID,
		math::Vector3 inDirection,
		float inRadInnerAngle,
		float inRadOuterAngle,
		const std::shared_ptr<Texture>& mask) :

		PointSphere(inRadiance, inRadius, inPosition, parentID),
		textureMask(mask),
		projection(math::Matrix::CreatePerspectiveFieldOfViewReversedLH(inRadOuterAngle * 2.0f, 1.0f, 0.1f, 100.f))
	{
		inDirection.Normalize(direction);
		SetAngleFalloff(inRadInnerAngle, inRadOuterAngle);
	}



	void SpotSphere::SetAngleFalloff(float innerAngle, float outerAngle)
	{
		float innerCosine = DirectX::XMScalarCos(innerAngle);
		float outerCosine = DirectX::XMScalarCos(outerAngle);

		angleScale = 1.0f / DirectX::XMMax<float>(math::EPSILON, (innerCosine - outerCosine));
	    angleOffset = -outerCosine * angleScale;
	}

	float SpotSphere::GetInnerAngle() const
	{
		return DirectX::XMScalarACos(angleScale + angleOffset / angleScale);
	}

	float SpotSphere::GetOuterAngle() const
	{
		return DirectX::XMScalarACos(-angleOffset / angleScale);
	}

	//void SpotSphere::SetAngleFalloff(float innerAngle, float outerAngle)
	//{
	//	float innerCosine = DirectX::XMScalarCos(innerAngle * 0.5f);
	//	float outerCosine = DirectX::XMScalarCos(outerAngle * 0.5f);

	//	innerCos = innerCosine;
	//	outerCos = outerCosine;
	//}

	//float SpotSphere::GetInnerAngle() const
	//{
	//	return DirectX::XMScalarACos(innerCos);
	//}

	//float SpotSphere::GetOuterAngle() const
	//{
	//	return DirectX::XMScalarACos(outerCos);
	//}
}