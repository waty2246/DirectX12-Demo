workspace "DirectX12-Demo"
    startproject "DirectX12-Demo"
    cppdialect "c++14"
    buildoptions "/std:c++14"

    system "windows"

    staticruntime "On"

    defines
    {
        "WIN32_LEAN_AND_MEAN"
    }

    flags 
    {
        "FatalWarnings",
    }

    configurations
    {
        "Debug",
        "Release",
    }

    platforms
    {
        "x64",
    }

    configuration "Debug"
        symbols "on"
        defines { "_DEBUG"}
        targetsuffix "_d"

    configuration "Release"
        defines { "NDEBUG"}
        optimize "on"

    project "DirectX12-Demo"
        language "C++"
        kind "WindowedApp"

        entrypoint "WinMainCRTStartup"
		
		pchheader "stdafx.h"
		pchsource "src/stdafx.cpp"
		
		nuget "WinPixEventRuntime:1.0.210818001"
		
		buildoptions {"/wd26451" }

        files
        {
			"premake5.lua",
            "src/**",
			"../shaders/*.hlsl",
        }
		
		filter("files:**.hlsl")
			flags("ExcludeFromBuild")
		filter {}
		
		vpaths 
		{ 
			["render"] = {"src/ShaderManager.*","*src/*Shader.*"},
			["config"] = 
			{
				"src/config.h",
				"src/stdafx.*",
			},
			["platform"] = 
			{
				"src/SystemApplication.*",
				"src/main.cpp",
				"src/Input.*",
			},
			["app"] = 
			{
				"src/Application.*",
				"src/PlayZone.*",
				"src/UserInterface.*",
			},
			["graphics"] = 
			{
				"src/D3D.*",
				"src/d3dx12.h",
			},
			["objects"] = 
			{
				"src/Terrain.*",
				"src/Skydome.*",
				"src/TerrainCell.*",
				"src/Fps.*",
				"src/Frustum.*",
				"src/Light.*",
				"src/GameTimer.*",
				"src/PlayerController.*",
				"src/Camera.*",
				
			},
			["ui"] = 
			{
				"src/Text.*",
				"src/Font.*",
				"src/Minimap.*",
				"src/Texture2DManager.*",
				"src/Image.*",
				"src/Texture2D.*",
				"src/DDSTextureLoader.*",
			},
			["_premake"] = {"premake5.lua"},
		}

		includedirs
		{
			"src/",
		}
		
		links
		{
			"dxgi.lib",
			"d3d12.lib",
			"d3dcompiler.lib",
			"winmm.lib",
		}
