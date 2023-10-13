project "fourth_Engine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")


	warnings "High"
	pchheader "pch.h"
	pchsource "pch.cpp"




	files
	{
		"fourthE.h",
		"pch.h",
		"pch.cpp",
		"src/**.c",
		"src/**.h",
		"src/**.c",
		"src/**.hpp",
		"src/**.cpp",
		"include/**.c",
		"include/**.h",
		"include/**.c",
		"include/**.hpp",
		"include/**.cpp",
		"vendor/SimpleMath/**.h",
		"vendor/SimpleMath/**.cpp",
		"vendor/SimpleMath/**.inl",
		"vendor/DirectXTex/DDSTextureLoader/**.h",
		"vendor/DirectXTex/DDSTextureLoader/**.cpp",


		"Shaders/**_PS.hlsl",
		"Shaders/**_VS.hlsl",
		"Shaders/**_HS.hlsl",
		"Shaders/**_GS.hlsl",
		"Shaders/**_DS.hlsl",
		"Shaders/**_CS.hlsl",
		"Shaders/**.hlsli"


	}

	
	includedirs
	{
		"vendor",
		"vendor/SimpleMath",
		"vendor/spdlog/include",

		"vendor/assimp/include",
		"vendor/DirectXTex/Include",
		"src",
		""
	}


	links
    {
        "spdlog",
        "assimp"
        -- "DirectXTex"
    }
	
	shadermodel("5.0")

	    shaderdefines(defineLightViewSpace)
		shaderdefines(defineParticlesLightning)
	


		shaderassembler("AssemblyCode")
		local shader_dir = "../".."bin/" .. outputdir .. "/fourth_Engine/Shaders/"

		 -- HLSL files that don't end with 'Extensions' will be ignored as they will be
         -- used as includes

		 --Compiled shaders go to each config target path
		filter("files:**.hlsl")
		   flags("ExcludeFromBuild")
		   shaderobjectfileoutput(shader_dir.."%{file.basename}"..".cso")
		   shaderassembleroutput(shader_dir.."%{file.basename}"..".asm")

		filter("files:**_PS.hlsl")
		   removeflags("ExcludeFromBuild")
		   shadertype("Pixel")

	    filter("files:**_GS.hlsl")
	    removeflags("ExcludeFromBuild")
	    shadertype("Geometry")

		filter("files:**_HS.hlsl")
		   removeflags("ExcludeFromBuild")
		   shadertype("Hull")

		filter("files:**_DS.hlsl")
		   removeflags("ExcludeFromBuild")
		   shadertype("Domain")

		filter("files:**_VS.hlsl")
		   removeflags("ExcludeFromBuild")
		   shadertype("Vertex")

		filter("files:**_CS.hlsl")
		   removeflags("ExcludeFromBuild")
		   shadertype("Compute")

			shaderoptions({"/WX"})


    
	filter "system:windows"

		systemversion "latest"
		staticruntime "on"
		flags {"MultiProcessorCompile"}

		defines
		{
			"ENGINE_PLATFORM_WINDOWS",
			"DEFAULT_CONSTRUCTOR",
			defineLightViewSpace,
			defineParticlesLightning,
			defineFFT_SHARED_MODEL_LOOP_CHANNELS,
			defineFFT_SHARED_MODEL_COMPUTE_RGB, 
			defineFFT_SHARED_MODEL,     
		}

  
		filter "configurations:Debug"
			defines "ENGINE_DEBUG"
			symbols "on"
		links
		{
			"%{wks.location}/%{prj.name}/vendor/DirectXTex/lib/bin_Debug/DirectXTex"
			-- "%{wks.location}/bin/Debug-windows-x86_64/assimp/zlibstaticd"

		}

		filter "configurations:Release"
			defines "ENGINE_RELEASE"
			optimize "on"
			symbols "on"

		links
		{
			"%{wks.location}/%{prj.name}/vendor/DirectXTex/lib/bin_Release/DirectXTex"
			-- "%{wks.location}/bin/Debug-windows-x86_64/assimp/zlibstaticd"
	
		}
		filter "configurations:Dist"
			defines "ENGINE_DIST"
			defines "NDEBUG"
			optimize "on"
		links
		{
			"%{wks.location}/%{prj.name}/vendor/DirectXTex/lib/bin_Release/DirectXTex"
			-- "%{wks.location}/bin/Debug-windows-x86_64/assimp/zlibstaticd"
	
		}