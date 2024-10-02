workspace "vkUtilExample"
	configurations {"Release", "Debug"}
	architecture "x64"
	language "C++"
    cppdialect "C++11"
	targetdir "build/bin/%{prj.name}/%{cfg.buildcfg}"
    objdir    "build/bin/objs"
    characterset("MBCS")
    buildoptions { "/EHsc" }
	location "build"
	project "vkUtilExample"
	kind "ConsoleApp"
	includedirs {
		"vendor/stb/",
		"C:/VulkanSDK/1.3.283.0/include/",
		"src/",
        "vendor/LWmath/",
        "vendor/",
        "vendor/imgui/"
	}
	files {
		"src/**.cpp",
		"src/**.h",
		"src/**.hpp",
		"premake5.lua",
		"src/**",
		"src/**",
        "vendor/nwin/*.cpp",
        "vendor/nwin/*.h",
        "vendor/imgui/*.cpp",
        "vendor/imgui/backends/*.cpp"
	}
	links {
        "dwmapi.lib",
		"C:/VulkanSDK/1.3.283.0/Lib/vulkan-1.lib"
	}
	filter "configurations:Release"
		optimize "On"
	filter "configurations:Debug"
		symbols "On"
