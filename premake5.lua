function generateCMakeDependencies()
   if not os.isfile("build/SDL3/CMakeCache.txt") then
      os.execute("cmake -S third_party/SDL -B build/SDL3")
   end
   if not os.isfile("build/SDL3/Debug/SDL3.dll") then
      os.execute("cmake --build build/SDL3 --config Debug")
   end
   if not os.isfile("build/SDL3/Release/SDL3.dll") then
      os.execute("cmake --build build/SDL3 --config Release")
   end

   if not os.isfile("build/box2c/CMakeCache.txt") then
      os.execute("cmake -S third_party/box2c -B build/box2c -DBOX2D_SAMPLES=0 -DBOX2D_UNIT_TESTS=0")
   end
   if not os.isfile("build/box2c/src/Debug/box2d.lib") then
      os.execute("cmake --build build/box2c --config Debug")
   end
   if not os.isfile("build/box2c/src/Release/box2d.lib") then
      os.execute("cmake --build build/box2c --config Release")
   end
end

--generateCMakeDependencies()

workspace "gothicJVM"
   configurations { "Debug", "Release" }
   location "build"
   externalanglebrackets "on"
   externalwarnings "off"
   architecture "x86_64"

project "JVM"
   kind "ConsoleApp"
   location "build/JVM"
   targetdir "%{prj.location}/%{cfg.buildcfg}"
   debugdir "%{prj.location}/%{cfg.buildcfg}"
   language "C++"
   cppdialect "C++20"

   files {
      "source/**.hpp",
      "source/**.cpp"
   }

   includedirs {
      "source"
   }

   --postbuildcommands {
   --   "{COPY} %[assets] %{prj.location}/%{cfg.buildcfg}/assets",
   --   "{COPYFILE} %[build/SDL3/%{cfg.buildcfg}/SDL3.dll] %{prj.location}/%{cfg.buildcfg}"
   --}

   filter "system:windows"
      staticruntime "off"
      systemversion "latest"
      defines { "WINDOWS" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "on"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "full"
