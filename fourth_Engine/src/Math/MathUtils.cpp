#include "pch.h"

#pragma once
#include "include/Math/MathUtils.h"
#include "include/Math/Ray.h"
#include <cmath>


using namespace DirectX;

namespace fth
{
	namespace math
	{

		bool IsEqual(float x, float y)
		{
			return std::fabsf(x - y) < EPSILON;
		}

		bool IsGreaterThanOrEqual(float x, float y)
		{
			return x > y || IsEqual(x, y);
		}

		bool IsLessThanOrEqual(float x, float y)
		{
			return x < y || IsEqual(x, y);;
		}

		float MillisecondsToSeconds(unsigned int milliseconds)
		{
			return 0.0f;
		}

		unsigned int GetPixelIndex(unsigned int width, unsigned int r, unsigned int c)
		{
			return r * width + c;
		}

		float Clamp(float val, float min, float max)
		{
			if (val > max)
			{
				return max;
			}
			if (val < min)
			{
				return min;
			}

			return val;
		}


		void InvertOrthonormalMatrix(const math::Matrix& source, math::Matrix& destination)
		{
			//To invert orthonormal, 
			//1-transpose 3x3 matrix and copy into destination
			math::Matrix temp{};

			SetMatrix3TransposedIntoMatrix4(source, temp);
			//2-extract translation row 
			math::Vector3 translationRow{ source.Translation() };
			//3-compute dot product of each normal basis vector with translation 
			math::Vector3 newTranslation{ translationRow.Dot(source.Right()), translationRow.Dot(source.Up()), translationRow.Dot(source.Forward()) };
			temp.Translation(-newTranslation);
			
			destination = temp;
		}

		void InvertOrthogonalMatrix(const math::Matrix& source, math::Matrix& destination)
		{
			//To invert orthogonal
			//1- Get inverse of the scale
			//2- Make destination matrix orthonormal
			//3- Transpose
			//4- Compute transform
			//5- Transform * Rotation
			//6- Rescale
			//Inverse scale

			math::Matrix temp{};

			math::SetMatrix3TransposedIntoMatrix4(source, temp);
			math::Vector3 scale{ 1.0f / source.Right().LengthSquared(), 1.0f / source.Up().LengthSquared(), 1.0f / source.Forward().LengthSquared() };

			math::Matrix scaleMatrix{ math::Matrix::CreateScale(scale) };

			temp *= scaleMatrix;

			math::Vector3 dot4thRow;
			dot4thRow.x = source.Translation().Dot(source.Right()) * scale.x;
			dot4thRow.y = source.Translation().Dot(source.Up()) * scale.y;
			dot4thRow.z = source.Translation().Dot(source.Forward()) * scale.z;

			temp.Translation(-dot4thRow);

			destination = temp;
		}



		void SetMatrix3IntoMatrix4(const SimpleMath::Matrix& newMatrix3, SimpleMath::Matrix& outMatrix)
		{
			outMatrix._11 = newMatrix3._11;
			outMatrix._12 = newMatrix3._12;
			outMatrix._13 = newMatrix3._13;
			outMatrix._21 = newMatrix3._21;
			outMatrix._22 = newMatrix3._22;
			outMatrix._23 = newMatrix3._23;
			outMatrix._31 = newMatrix3._31;
			outMatrix._32 = newMatrix3._32;
			outMatrix._33 = newMatrix3._33;
			outMatrix._44 = 1.0f;
		}

		void SetMatrix3TransposedIntoMatrix4(const SimpleMath::Matrix& newMatrix3, SimpleMath::Matrix& outMatrix)
		{
			outMatrix._11 = newMatrix3._11;
			outMatrix._12 = newMatrix3._21;
			outMatrix._13 = newMatrix3._31;
			outMatrix._21 = newMatrix3._12;
			outMatrix._22 = newMatrix3._22;
			outMatrix._23 = newMatrix3._32;
			outMatrix._31 = newMatrix3._13;
			outMatrix._32 = newMatrix3._23;
			outMatrix._33 = newMatrix3._33;
			outMatrix._44 = 1.0f;
		}
		void ClampAndRemapColorTo255(DirectX::SimpleMath::Color& colorToRemap)
		{
			colorToRemap.Saturate(), colorToRemap * 255.0f;
		}

		math::Matrix UpdateTransformMatrixScaled(const Transform& transform)
		{
			return math::Matrix::CreateScale(transform.scale) * math::Matrix::CreateFromQuaternion(transform.rotation) * math::Matrix::CreateTranslation(transform.position);
		}

		void UpdateTransformMatrixScaled(const Transform& transform, math::Matrix& outTransformMatrix)
		{
			math::Matrix rotationMatrix{ math::Matrix::CreateFromQuaternion(transform.rotation) };

			outTransformMatrix = math::Matrix::CreateScale(transform.scale) * rotationMatrix * math::Matrix::CreateTranslation(transform.position);
		}

		void UpdateTransformMatrixScaled(const Transform& transform, math::Matrix& outTransformMatrix, math::Matrix& outInverseTransformMatrix)
		{
			math::Matrix rotationMatrix{ math::Matrix::CreateFromQuaternion(transform.rotation)};
	
			//First update transform matrix
			outTransformMatrix = math::Matrix::CreateScale(transform.scale) * rotationMatrix * math::Matrix::CreateTranslation(transform.position);

			//Second, calculate inverse transform matrix based on the first one
			math::Vector3 dot4thRow;
			dot4thRow.x = transform.position.Dot(rotationMatrix.Right());
			dot4thRow.y = transform.position.Dot(rotationMatrix.Up());
			dot4thRow.z =  transform.position.Dot(rotationMatrix.Forward());

			outInverseTransformMatrix.Translation(-dot4thRow);
			SetMatrix3TransposedIntoMatrix4(rotationMatrix, outInverseTransformMatrix);


			outInverseTransformMatrix *= math::Matrix::CreateScale(1.0f / transform.scale.x, 1.0f / transform.scale.y, 1.0f / transform.scale.z);
			
		}

		void ResetMatrixColumn(uint8_t column, math::Matrix& src)
		{
			float* dst = &src._11;
			for (uint8_t idx = 0; idx < 4; ++idx)
			{
				*(dst + column + (idx * 4)) = 0.0f;
			}
		}

		void UpdateTransformMatrixUnscaled(const Transform& transform, math::Matrix& outTransformMatrix)
		{
			math::Matrix rotationMatrix{  };

			outTransformMatrix = math::Matrix::CreateFromQuaternion(transform.rotation);
			outTransformMatrix.Translation(transform.position);
		}

		math::Matrix UpdateTransformMatrixUnscaled(const Transform& transform)
		{
			math::Matrix returnMatrix{ math::Matrix::CreateFromQuaternion(transform.rotation) };
			returnMatrix.Translation(transform.position);

			return returnMatrix;
		}

		void UpdateTransformMatrixUnscaled(const Transform& transform, math::Matrix& outTransformMatrix, math::Matrix& outInverseTransformMatrix)
		{
			math::Matrix rotationMatrix{ math::Matrix::CreateFromQuaternion(transform.rotation) };

			//First update inverse matrix
			outTransformMatrix.Translation(transform.position);
			SetMatrix3IntoMatrix4(rotationMatrix, outTransformMatrix);
			//Second, calculate transform matrix based on the first one

			InvertOrthonormalMatrix(outTransformMatrix, outInverseTransformMatrix);

		}


		Ray TransformRay(const Ray& inRay, const math::Matrix& spaceTransform)
		{
			math::Vector3 origin{ TransformPoint(inRay.position, spaceTransform) };
			math::Vector3 direction{ TransformVector(inRay.direction, spaceTransform) };
			direction.Normalize();
			return {origin, direction};
		}
		math::Vector3 TransformPoint(const math::Vector3& inPoint, const math::Matrix& spaceTransform)
		{
			return DirectX::XMVector4Transform(math::Vector4(inPoint.x, inPoint.y, inPoint.z, 1.0f), spaceTransform);
		}
		math::Vector3 TransformVector(const math::Vector3& inVector, const math::Matrix& spaceTransform)
		{
			return DirectX::XMVector4Transform(math::Vector4(inVector.x, inVector.y, inVector.z, 0.0f), spaceTransform);
		}


		constexpr math::Vector4 clip[8]
		{
			{ -1.0f,  1.0f,  1.0f, 1.0f },
			{  1.0f,  1.0f,  1.0f, 1.0f },
			{  1.0f, -1.0f,  1.0f, 1.0f },
			{ -1.0f, -1.0f,  1.0f, 1.0f },

			{ -1.0f,  1.0f,  0.0f, 1.0f },
			{  1.0f,  1.0f,  0.0f, 1.0f },
			{  1.0f, -1.0f,  0.0f, 1.0f },
			{ -1.0f, -1.0f,  0.0f, 1.0f }
		};

		void Frustum::ComputeFrustumCorners(const math::Matrix& viewProj)
		{
			math::Vector4 output[8];
			DirectX::XMVector4TransformStream(output, sizeof(DirectX::XMFLOAT4), clip, sizeof(DirectX::XMFLOAT4), 8, viewProj);


			for (uint8_t i = 0; i < 8; ++i)
			{
			     corners[i] = *reinterpret_cast<math::Vector3*>(&(output[i] / output[i].w));
			}

		}

		void Frustum::ComputeFrustumCornersDirections(const math::Matrix& invView, const math::Matrix& invProj)
		{
			math::Vector4 output[4];
			DirectX::XMVector4TransformStream(output, sizeof(DirectX::XMFLOAT4), clip, sizeof(DirectX::XMFLOAT4), 4, invProj);

			for (uint8_t i = 0; i < 4; ++i)
			{
				output[i] = (output[i] / output[i].w);
				output[i].w = 0.0f;
			}
			DirectX::XMVector4TransformStream(output, sizeof(DirectX::XMFLOAT4), output, sizeof(DirectX::XMFLOAT4), 4, invView);

			for (uint8_t i = 0; i < 4; ++i)
			{
				output[i].Normalize();
				cornerDirections[i] = *reinterpret_cast<math::Vector3*>(&output[i]);
			}
		}


	}


}
