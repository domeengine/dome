workspace "wren"
  configurations { "Release", "Debug" }
  platforms { "64bit", "32bit", "64bit-no-nan-tagging", "arm"}
  defaultplatform "64bit"
  startproject "wren_test"
  location ("../../lib/wren/projects/" .. _ACTION)

  filter "configurations:Debug"
    targetsuffix "_d"
    defines { "DEBUG" }
    symbols "On"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"

  filter "platforms:64bit-no-nan-tagging"
    defines { "WREN_NAN_TAGGING=0" }

  --the 'xcode4' and 'gmake2' folder names
  --are simply confusing, so, simplify then
  filter { "action:xcode4" }
    location ("../../lib/wren/projects/xcode")

  filter "action:gmake2"
    location ("../../lib/wren/projects/make")

  filter { "action:gmake2", "system:bsd" }
    location ("../../lib/wren/projects/make.bsd")

  filter { "action:gmake2", "system:macosx" }
    location ("../../lib/wren/projects/make.mac")

  filter "platforms:32bit"
    architecture "x86"

  filter "platforms:64bit"
    architecture "x86_64"

  filter "platforms:arm"
    architecture "aarch64"

  filter "system:windows"
    systemversion "latest"
    defines { "_CRT_SECURE_NO_WARNINGS" }

  filter "system:linux"
    links { "m" }

  filter "system:bsd"
    links { "m" }

project "wren"
  kind "StaticLib"
  language "C"
  cdialect "C99"
  targetdir "../../lib/wren/lib"

  files {
    "../../lib/wren/src/**.h",
    "../../lib/wren/src/**.c"
  }

  includedirs {
    "../../lib/wren/src/include",
    "../../lib/wren/src/vm",
    "../../lib/wren/src/optional"
  }

project "wren_shared"
  kind "SharedLib"
  targetname "wren"
  language "C"
  cdialect "C99"
  targetdir "../../lib/wren/lib"

  files {
    "../../lib/wren/src/**.h",
    "../../lib/wren/src/**.c"
  }

  includedirs {
    "../../lib/wren/src/include",
    "../../lib/wren/src/vm",
    "../../lib/wren/src/optional"
  }

project "wren_test"
  kind "ConsoleApp"
  language "C"
  cdialect "C99"
  targetdir "../../lib/wren/bin"
  dependson "wren"
  links { "wren" }

  files {
    "../../lib/wren/test/main.c",
    "../../lib/wren/test/test.c",
    "../../lib/wren/test/test.h",
    "../../lib/wren/test/api/*.c",
    "../../lib/wren/test/api/*.h"
  }

  includedirs {
    "../../lib/wren/src/include"
  }
