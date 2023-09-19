#include "pch.h"

#pragma once
#include "include/Math/Vec3D.h"
#include "include/Math/MathUtils.h"
#include <assert.h>

namespace fth

{
    const Vec3D Vec3D::Zero;
    const Vec3D Vec3D::Up{ 0.0f, 1.0f, 0.0f };
    const Vec3D Vec3D::Down{ 0.0f, -1.0f, 0.0f };
    const Vec3D Vec3D::Left{ -1.0f, 0.0f, 0.0f };
    const Vec3D Vec3D::Right{ 1.0f, 0.0f, 0.0f };
    const Vec3D Vec3D::Forward{ 0.0f, 0.0f, 1.0f };
    const Vec3D Vec3D::Backwards{ 0.0f, 0.0f, -1.0f };

    Vec3D::Vec3D(float x, float y, float z) :
        mX(x),
        mY(y),
        mZ(z)
    {

    }

    Vec3D::Vec3D() :
        Vec3D(0.0f, 0.0f, 0.0f)
    {

    }

    std::ostream& operator<<(std::ostream& stream, const Vec3D& vec3D)
    {
        stream << "X: " << vec3D.mX << "\tY: " << vec3D.mY << "\tZ: " << vec3D.mZ << std::endl;

        return stream;
    }

    Vec3D operator*(float scale, const Vec3D& vec)
    {
        return vec * scale;
    }

    bool Vec3D::operator==(const Vec3D& otherVec) const
    {
        return math::IsEqual(mX, otherVec.mX) && math::IsEqual(mY, otherVec.mY) && math::IsEqual(mZ, otherVec.mZ);
    }

    bool Vec3D::operator!=(const Vec3D& otherVec) const
    {
        return !(*this == otherVec);
    }

    Vec3D Vec3D::operator+(const Vec3D& otherVec) const
    {
        return Vec3D(mX + otherVec.mX, mY + otherVec.mY, mZ + otherVec.mZ);
    }

    Vec3D Vec3D::operator-(const Vec3D& otherVec) const
    {
        return Vec3D(mX - otherVec.mX, mY - otherVec.mY, mZ - otherVec.mZ);
    }

    Vec3D& Vec3D::operator-=(const Vec3D& otherVec)
    {
        *this = *this - otherVec;
        return *this;
    }

    Vec3D& Vec3D::operator+=(const Vec3D& otherVec)
    {
        *this = *this + otherVec;
        return *this;
    }

    Vec3D Vec3D::operator-() const
    {
        return Vec3D(-mX, -mY, -mZ);;
    }

    Vec3D Vec3D::operator*(float scale) const
    {
        return Vec3D(scale * mX, scale * mY, scale * mZ);
    }

    Vec3D Vec3D::operator/(float scale) const
    {
        bool isNull{ !(std::fabsf(scale) > math::EPSILON) };
        assert(!isNull && "/ operation by 0 on Vec3D");

        if (isNull)
        {
            return Vec3D::Zero;
        }
        return Vec3D(mX / scale, mY / scale, mZ / scale);
    }

    Vec3D& Vec3D::operator*=(float scale)
    {
        *this = *this * scale;
        return *this;
    }

    Vec3D& Vec3D::operator/=(float scale)
    {
        *this = *this / scale;
        return *this;

    }

    float Vec3D::Mag2() const
    {
        return Dot(*this);
    }

    float Vec3D::Mag() const
    {
        return std::sqrt(Mag2());
    }

    Vec3D Vec3D::GetUnitVec() const
    {
        float mag = Mag();

        if (mag > math::EPSILON)
            return *this / mag;

        return Vec3D::Zero;
    }

    Vec3D& Vec3D::Normalize()
    {
        float mag = Mag();

        if (mag > math::EPSILON)
            *this /= mag;
        else
            *this = Vec3D::Zero;

        return *this;

    }


    //For points...
    float Vec3D::Distance(const Vec3D& vec) const
    {
        return 0.0f;
    }

    float Vec3D::Dot(const Vec3D& vec) const
    {
        return mX * vec.mX + mY * vec.mY + mZ * vec.mZ;
    }

    Vec3D Vec3D::Cross(const Vec3D& vec) const
    {
        float crossX{ mY * vec.mZ - mZ * vec.mY };
        float crossY{ mZ * vec.mX - mX * vec.mZ };
        float crossZ{ mX * vec.mY - mY * vec.mX };
        return Vec3D(crossX, crossY, crossZ);
    }

    Vec3D Vec3D::ProjectOnto(const Vec3D& vec2) const
    {
        return Vec3D();
    }

    float Vec3D::AngleBetween(const Vec3D& vec2) const
    {
        return 0.0f;
    }

    Vec3D Vec3D::Reflect(const Vec3D& normal) const
    {
        return Vec3D();
    }

    void Vec3D::Rotate(float angle, const Vec3D& aroundPoint)
    {
    }

    Vec3D Vec3D::RotationResult(float angle, const Vec3D& aroundPoint) const
    {
        return Vec3D();
    }

}