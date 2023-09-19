#pragma once

//this header is meant to be only used by client applications

//App
#include "include/Win32Headers.h"
#include "include/App.h"
#include "include/Win32Window.h"
#include "include/Specifications.h"
//Add EntryPoint preprocessor define?
#include "fth_EntryPoint.h"
#include "include/Win32Window.h"
#include "include/Render/Renderer/D3DRenderer.h"
#include "include/Input/InputController.h"
#include "include/Input/BitsetController.h"
#include "include/Render/Lights.h"
#include "include/Managers/TextureManager.h"
#include "include/Render/Model.h"
#include "include/Systems/MeshSystem.h"
#include "include/Systems/TransformSystem.h"
#include "include/Systems/LightSystem.h"
#include "include/Systems/ParticleSystem.h"
#include "include/Systems/DecalSystem.h"
#include "include/Managers/ModelManager.h"
#include "include/Render/Renderer/PostProcess.h"
#include "include/Tools/IBLReflectionCapture.h"
#include "include/Utils/Random.h"
#include "include/Render/Atlas.h"
//Logging
#include "include/Logging/LogSystem.h"
#include "include/Logging/ClientLogMacros.h"

//Assertion
#include "include/Assert.h"
#include "include/D3DAssert.h"


//Maths
#include "include/Math/Maths.h"
#include "include/Math/MathUtils.h"
#include "include/Math/Ray.h"
#include "include/Render/Dragger.h"


//Primitive shapes
#include "include/Render/Primitive Shapes/Sphere.h"
#include "include/Render/Primitive Shapes/Plane.h"
#include "include/Managers/CameraManager.h"