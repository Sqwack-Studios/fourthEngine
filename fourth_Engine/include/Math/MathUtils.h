#pragma once
namespace fth
{

	//TODO: Wrap SimpleMath library and add redefinitions to forward vector handiness


	struct Ray;

	namespace math
	{
		
		static constexpr float EPSILON = 0.0001f;
		static constexpr float PI = 3.141593f;
		static constexpr float TWO_PI = 2.0f * PI;

		bool IsEqual(float x, float y);

		bool IsGreaterThanOrEqual(float x, float y);

		bool IsLessThanOrEqual(float x, float y);

		float MillisecondsToSeconds(unsigned int milliseconds);

		unsigned int GetPixelIndex(unsigned int width, unsigned int r, unsigned int c);

		float Clamp(float val, float min, float max);


		//Matrix operations
		void InvertOrthogonalMatrix(const math::Matrix& source, math::Matrix& destination);
		void InvertOrthonormalMatrix(const math::Matrix& source, math::Matrix& destination);
	    void SetMatrix3IntoMatrix4(const DirectX::SimpleMath::Matrix& newMatrix3, DirectX::SimpleMath::Matrix& outMatrix);
		void SetMatrix3TransposedIntoMatrix4(const DirectX::SimpleMath::Matrix& newMatrix3, DirectX::SimpleMath::Matrix& outMatrix);
		math::Matrix UpdateTransformMatrixScaled(const Transform& transform);
		void UpdateTransformMatrixScaled(const Transform& transform, math::Matrix& outTransformMatrix);
		void UpdateTransformMatrixScaled(const Transform& transform, math::Matrix& outTransformMatrix, math::Matrix& outInverseTransformMatrix);

		void ResetMatrixColumn(uint8_t column, math::Matrix& src);
		void UpdateTransformMatrixUnscaled(const Transform& transform, math::Matrix& outTransformMatrix);
		math::Matrix UpdateTransformMatrixUnscaled(const Transform& transform);
		void UpdateTransformMatrixUnscaled(const Transform& transform, math::Matrix& outTransformMatrix, math::Matrix& outInverseTransformMatrix);

		Ray TransformRay(const Ray& inRay, const math::Matrix& spaceTransform);
		math::Vector3 TransformPoint(const math::Vector3& inPoint, const math::Matrix& spaceTransform);
		math::Vector3 TransformVector(const math::Vector3& inVector, const math::Matrix& spaceTransform);



//     Near    Far
//    0----1  4----5
//    |    |  |    |
//    |    |  |    |
//    3----2  7----6
		struct Frustum
		{
			void ComputeFrustumCorners(const math::Matrix& viewProj);
			void ComputeFrustumCornersDirections(const math::Matrix& invView, const math::Matrix& invProj);

			const math::Vector3& getFront_TL() const { return corners[0]; }
			const math::Vector3& getFront_TR() const { return corners[1]; }
			const math::Vector3& getFront_BR() const { return corners[2]; }
			const math::Vector3& getFront_BL() const { return corners[3]; }

			const math::Vector3& getBack_TL() const { return corners[4]; }
			const math::Vector3& getBack_TR() const { return corners[5]; }
			const math::Vector3& getBack_BR() const { return corners[6]; }
			const math::Vector3& getBack_BL() const { return corners[7]; }

			const math::Vector3& get_TL_Dir() const { return cornerDirections[0]; }
			const math::Vector3& get_TR_Dir() const { return cornerDirections[1]; }
			const math::Vector3& get_BR_Dir() const { return cornerDirections[2]; }
			const math::Vector3& get_BL_Dir() const { return cornerDirections[3]; }

														 
			math::Vector3 corners[8];
			math::Vector3 cornerDirections[4];
		};

		//Color operations
		void ClampAndRemapColorTo255(DirectX::SimpleMath::Color& colorToRemap);
	}


}