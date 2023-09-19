#pragma once
#include "include/Render/Camera.h"

namespace fth
{
	class CameraManager
	{
	public:
		static renderer::Camera& GetActiveCamera();
		//static void AddCamera();
		//static void SetActiveCamera();

	};
}