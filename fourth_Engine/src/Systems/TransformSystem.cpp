#include "pch.h"

#pragma once
#include "include/Systems/TransformSystem.h"


namespace fth
{ 
    TransformSystem::TransformSystem() {}

    void TransformSystem::Init()
    {
        m_transforms.reserve(static_cast<size_t>(INITIAL_POOL));
    }
    void TransformSystem::Shutdown()
    {
        m_transforms.clear();
    }

    Handle<math::Matrix> TransformSystem::AddTransform(const math::Matrix& transformMatrix)
    { 
        return { m_transforms.insert(transformMatrix) };
    }

    Handle<math::Matrix> TransformSystem::AddTransform(const math::Transform& transform)
    { 
        return { m_transforms.insert(math::UpdateTransformMatrixScaled(transform)) };
    }
    const math::Matrix& TransformSystem::QueryTransformMatrix(Handle<math::Matrix>  id) const
    { 
        return m_transforms[id];
    }
    math::Matrix& TransformSystem::QueryTransformMatrix(Handle<math::Matrix>  id)
    { 
        return m_transforms[id];
    }


    void TransformSystem::EraseTransform(Handle<math::Matrix> id)
    { 
        m_transforms.erase(id.id); 
    }

}