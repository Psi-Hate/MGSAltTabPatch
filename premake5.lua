workspace "MGSALTTABPatch"
   configurations { "Debug", "Release" }
   architecture "x64"
   location "build"
   buildoptions {"-std:c++latest"}
   
   defines { "X64" }
     
project "MGSALTTABPatch"
   kind "SharedLib"
   language "C++"
   targetdir "bin/x64/%{cfg.buildcfg}"
   targetname "MGSALTTABPatch"
   targetextension ".asi"
   
   includedirs { "source" }
   includedirs { "external" }
   
   files { "source/dllmain.h", "source/dllmain.cpp", "source/codereplacement.cpp", "source/codereplacement.h", "source/mgs2.cpp", "source/mgs2.h", "source/mgs3.cpp", "source/mgs3.h", "external/Hooking.Patterns/Hooking.Patterns.cpp", "external/Hooking.Patterns/Hooking.Patterns.h" }
   
   characterset ("UNICODE")
   
   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
      staticruntime "On"
      