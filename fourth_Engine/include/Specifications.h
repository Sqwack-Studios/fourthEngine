#include "pch.h"

#pragma once
#include "dxgiformat.h"

namespace fth
{

	struct EngineAppSpecs
	{
		std::wstring       appName{ L"fourth Engine" };
		uint16_t           windowWidth{ 1600 };
		uint16_t           windowHeight{ 900 };
		bool               fullscreen{ false };
		bool               resizable{ true };
		bool               startMaximized{ false };
		uint16_t           targetFPS{ 60 };//frametime is 1/targetFPS
		uint16_t           multisamples{ 1 };
		float              backBufferSizeMultiplier { 1.0f };
	};

	struct WindowSpecs
	{
		std::wstring       windowName{ L"fourth Engine" };
		uint16_t           windowWidth{ 640 };
		uint16_t           windowHeight{ 360 };
		bool               fullscreen{ false };
		float              backBufferSizeMultiplier{ 0.5f };
	};


	struct Config
	{
		//Paths
		static constexpr std::wstring_view wAssetsPath = L"../../../Assets/";
		static constexpr std::string_view assetsPath = "../../../Assets/";


		//Rendering
		static constexpr uint32_t MAX_POINT_LIGHTS = 20;
		static constexpr uint32_t MAX_DIRECITONAL_LIGHTS = 1;
		static constexpr uint32_t MAX_SPOT_LIGHTS = 1;
		//
		static constexpr uint32_t MAX_SHADOWMAP_TEXTURES = MAX_POINT_LIGHTS * 6 + MAX_DIRECITONAL_LIGHTS + MAX_SPOT_LIGHTS;

	};



}