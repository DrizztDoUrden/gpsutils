project "texttest"
	location "."
    kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	files { "src/**.cpp", "src/include/**.hpp", "include/**.hpp", "premake5.lua" }
	includedirs { "src/include" }
	GPSUtils:Link()
