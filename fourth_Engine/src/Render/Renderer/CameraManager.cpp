#include "pch.h"

#pragma once
#include "include/Managers/CameraManager.h"
#include "include/Render/Renderer/D3DRenderer.h"

namespace fth
{
	renderer::Camera& CameraManager::GetActiveCamera()
	{
		return renderer::D3DRenderer::Get().GetActiveCamera();
	}


}