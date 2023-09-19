#pragma once
#include <iostream>

//temporary, I guess we'll be using DirectXMath. I already had this class written so im reusing it

namespace fth
{



	class Vec3D
	{
	public:

		Vec3D();
		Vec3D(float x, float y, float z);

		static const Vec3D Zero;
		static const Vec3D Up;
		static const Vec3D Down;
		static const Vec3D Left;
		static const Vec3D Right;
		static const Vec3D Forward;
		static const Vec3D Backwards;

		friend std::ostream& operator<<(std::ostream& stream, const Vec3D& vec3D);

		inline void SetX(float x) { mX = x; }
		inline void SetY(float y) { mY = y; }
		inline void SetZ(float z) { mZ = z; }

		inline const float& GetX() const { return mX; }
		inline const float& GetY() const { return mY; }
		inline const float& GetZ() const { return mZ; }

		//operator overloading

		bool operator== (const Vec3D& otherVec) const;
		bool operator!= (const Vec3D& otherVec) const;

		Vec3D operator+(const Vec3D& otherVec) const;
		Vec3D operator-(const Vec3D& otherVec) const;
		Vec3D& operator-=(const Vec3D& otherVec);
		Vec3D& operator+=(const Vec3D& otherVec);

		Vec3D operator-() const;
		Vec3D operator*(float scale) const;
		Vec3D operator/(float scale) const;
		Vec3D& operator*=(float scale);
		Vec3D& operator/=(float scale);

		float Mag2() const;
		float Mag() const;

		Vec3D GetUnitVec() const;
		Vec3D& Normalize();

		float Distance(const Vec3D& vec) const;
		float Dot(const Vec3D& vec) const;
		Vec3D Cross(const Vec3D& vec) const;
		Vec3D ProjectOnto(const Vec3D& vec2) const;
		float AngleBetween(const Vec3D& vec2) const;

		Vec3D Reflect(const Vec3D& normal) const;

		void Rotate(float angle, const Vec3D& aroundPoint);
		Vec3D RotationResult(float angle, const Vec3D& aroundPoint) const;

		friend Vec3D operator*(float scale, const Vec3D& vec);

	private:

		float mX, mY, mZ;
	};

}