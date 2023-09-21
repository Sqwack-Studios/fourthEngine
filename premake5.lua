include "fourth_Engine/vendor/_0dependencies_premake5.lua"


workspace "Render_Engine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	startproject "Client"

	defines
	{
		"SPDLOG_COMPILED_LIB",
        "_CRT_SECURE_NO_WARNINGS"
	}
	
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
defineLightViewSpace = "ILLUMINATION_VIEW_SPACE"

defineLightViewSpace = ""	
defineLightViewSpace = "CAMERA_CENTER_WS"   

defineParticlesLightning = "PARTICLES_LIGHTNING_SPHERES"
defineParticlesLightning = "PARTICLES_LIGHTNING_BILLBOARDS"






group "Dependencies"
	include "fourth_Engine/vendor/spdlog"
	include "fourth_Engine/vendor/assimp"	

group "Engine"
	include "fourth_Engine/fth_premake5.lua"





project "Client"

	location "Client"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")
	debugdir ("bin/" .. outputdir .. "/%{prj.name}")


	warnings "High"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/include/**.h",
		"%{prj.name}/include/**.cpp"

	}

	includedirs
	{
		"%{prj.name}",
		"fourth_Engine/vendor/spdlog/include/",
		"fourth_Engine/vendor/assimp/include/",
		"fourth_Engine/vendor/assimp/code/",



		"fourth_Engine/vendor",
		"fourth_Engine/vendor/DirectXTex/DDSTextureLoader",
		"fourth_Engine/vendor/DirectXTex/Include",
	    "fourth_Engine/vendor/SimpleMath",
		"fourth_Engine"
	}

	links
	{
		"assimp",
		"fourth_Engine"
	}

		filter "system:windows"

		systemversion "latest"
		staticruntime "on"
		flags {"MultiProcessorCompile"}



		filter "configurations:Debug"
			defines "ENGINE_DEBUG"
			symbols "on"
			links
			{
				"%{wks.location}/%{prj.name}/vendor/ImGUI/lib/bin_Debug/ImGui"
			}

		filter "configurations:Release"
			defines "ENGINE_RELEASE"
			optimize "on"
			symbols "on"
			links
			{
				"%{wks.location}/%{prj.name}/vendor/ImGUI/lib/bin_Release/ImGui"
			}
			

		filter "configurations:Dist"
			defines "ENGINE_DIST"
			optimize "on"
			links
			{
				"%{wks.location}/%{prj.name}/vendor/ImGUI/lib/bin_Release/ImGui"
			}
		

project "IBLGenerator"

	location "IBLGenerator"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir("bin-int/" .. outputdir .. "/%{prj.name}")
	debugdir ("bin/" .. outputdir .. "/%{prj.name}")


	warnings "High"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"fourth_Engine",
		"fourth_Engine/vendor"
	}

	links
	{
		"fourth_Engine"
	}

	filter "system:windows"

	systemversion "latest";
	staticruntime "on"
	flags {"MultiProcessorCompile"}

	filter "configurations:Debug"
	symbols "on"

	filter "configurations:Release"
	optimize "on"

