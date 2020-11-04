srcdir = "../src/"
libdir = "../src/lib/"
sdlConfig = "`which sdl2-config 1>/dev/null && echo \"sdl2-config\" || echo \""..libdir.."sdl2-config\"`"
version = os.outputof("git describe --tags")
hash = os.outputof("git rev-parse --short HEAD")

workspace "DOME"
  configurations { "debug", "release" }

project "DOME"
  kind "WindowedApp"
  language "C"
  cdialect "c99"
  targetname "dome"
  targetextension ""
  targetdir "."

  files { srcdir.."main.c" }
  sysincludedirs { srcdir.."include", srcdir.."include/SDL2" }

  defines { 
    "DOME_VERSION=\""..version.."\"",
    "HASH=\""..hash.."\""
  }

  enablewarnings {
    "all", "extra"

  }
  disablewarnings {
    "unused-parameter",
    "unused-value",
    "unused-function",
    "incompatible-pointer-types-discards-qualifiers"
  }

  buildoptions { 
    "-pedantic",
    "`sdl2-config --cflags`",
    "-mmacosx-version-min=10.14"
  }

  postbuildcommands {
    "{RMDIR} obj"

  }


  staticruntime "On"
  syslibdirs { srcdir.."lib" }
  links { "wren", "m" }
  linkoptions { 
    "`sdl2-config --static-libs`"
  }

  filter "configurations:Debug"
    defines { "DEBUG=1" }
    symbols "On"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"
