#pragma once
#include "include/Systems/TransformSystem.h"

//Why does the light need to know its transform ID? It doesn't make sense. This is why we have systems, to query data we need, and why entities exist in ECS,
//to store the components IDs

namespace fth
{
	class Texture;
}
namespace fth::lights
{
	//Physical lights
	struct DirectionalSphere;
	struct PointSphere;
	struct SpotSphere;

	struct cbDirectSphereGPU;
	struct cbPointSphereGPU;
	struct cbSpotSphereGPU;

	//Don't modify transform parameters directly, use transform ID.
	struct DirectionalSphere
	{
		DirectionalSphere(math::Vector3 luminance, math::Vector3 direction, float solidAngle, Handle<math::Matrix> parentID);

		void SetLuminance(const math::Vector3& newColor) { luminance = newColor; }

	public:
		math::Vector3  luminance;
		float          solidAngle;

		math::Vector3  direction;

		Handle<math::Matrix>    parentID;

     
	};

	struct cbDirectSphereGPU
	{
		math::Vector3  luminance;
		float          solidAngle;

		math::Vector3  direction;
		float          pad[1];
	};


	struct PointSphere
	{
		PointSphere(math::Vector3 radiance, float radius, math::Vector3 position, Handle<math::Matrix> parentID);


		void UpdateRadiance(math::Vector3 inRadiance) { radiance = inRadiance; };
		void UpdateRadiance(math::Vector3 luminousFlux, float radius);
	
		math::Vector3    radiance;
		float            radius;
					     
		math::Vector3    position;


		
		Handle<math::Matrix>      parentID;
	};

	struct cbPointSphereGPU
	{
		math::Vector3 luminance;
		float         radius;

		math::Vector3 position;
		float         pad[1];
	};

	struct SpotSphere: public PointSphere
	{
		SpotSphere(
			math::Vector3 radiance, 
			float radius, 
			math::Vector3 position,
			Handle<math::Matrix> parentID,
			math::Vector3 direction, 
			float radInnerAngle,
			float radOuterAngle,
			const std::shared_ptr<Texture>& mask
			);


		//Provide angles in radians
		void SetAngleFalloff(float innerAngle, float outerAngle);
		//Returns inner angle in radians
		float GetInnerAngle() const;
		//Returns outer angle in radians
		float GetOuterAngle() const;
			
	public:
		float           angleScale;

		math::Vector3   direction;
		float           angleOffset;

		math::Matrix    projection;

		std::shared_ptr<Texture> textureMask = nullptr;
	};

	struct cbSpotSphereGPU
	{
		math::Vector3 luminance;
		float         radius;

		math::Vector3 position;
		float         angleScale;

		math::Vector3 orientation;
		float         angleOffset;

		math::Matrix  viewProjection;
	};
}