if not srcRoot then
	srcRoot = _SCRIPT_DIR .. "/.."
end

Version = include "Version.lua"
LibHelper = include "LibHelper.lua"

local name = "gpsutils"

if not solutionDeclared then
	solution "gpsutils"
		configurations { "Debug", "Release" }
		targetdir "bin/%{cfg.platform}/%{cfg.buildcfg}"
		objdir "obj/%{prj.name}/%{cfg.platform}/%{cfg.buildcfg}"
		editorintegration "On"

		configurations { "Debug", "Release" }
		platforms { "x64", "x32" }
	
		filter "configurations:Debug"
			defines { "DEBUG" }
			symbols "On"
			optimize "Off"
		filter "configurations:Release"
			defines { "NDEBUG" }
			optimize "On"
			symbols "Off"
		filter "platforms:x64"
			architecture "x64"
		filter "platforms:x32"
			architecture "x32"
		filter {}

	    filter "action:vs*"
		    buildoptions "/std:c++latest"
		    defines { "WINDOWS", "VISUAL_STUDIO" }
		    systemversion "10.0.15063.0"
end

GPSUtils = LibHelper(name)

project "gpsutils"
	location "."
	language "C++"
	cppdialect "C++17"
	files { "src/**.cpp", "src/include/**.hpp", "include/**.hpp", "include/**.h", "*.lua" }
	includedirs { "src/include" }
	kind "SharedLib"
	defines "GPSU_EXPORT"
	GPSUtils:Include()

	filter "system:not windows"
		links { "dl", "stdc++fs" }
	filter {}

include "tests/pool"
include "tests/text"
