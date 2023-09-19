#include "pch.h"

#pragma once
#include "include/Render/Dragger.h"
#include "include/Render/Renderer/D3D11Headers.h"
#include "include/Render/Primitive Shapes/Sphere.h"

namespace fth
{

	void ModelDragger::drag(const math::Vector3 & offset)
	{

		if (!target.isValid())
			return;

			math::Matrix& targetMatrix = TransformSystem::Get().QueryTransformMatrix(target);
			math::Vector3 currentPosition = targetMatrix.Translation();
			targetMatrix.Translation(currentPosition + offset);
		
	}


}