srcdir = "../src/"
libdir = "src/lib/"
sdlConfig = "which sdl2-config 1>/dev/null && echo \"sdl2-config\" || echo \""..libdir.."sdl2-config\""
-- sdlConfig = libdir.."sdl2-config"
version = os.outputof("git describe --tags")
hash = os.outputof("git rev-parse --short HEAD")

workspace "DOME"
  configurations { "Release", "Debug" }
  platforms { "static64", "static32", "shared64" }
  defaultplatform "static64"
  location ".."
  basedir ".."


project "DOME"
  kind "WindowedApp"
  language "C"
  cdialect "c99"
  targetdir ".."
  targetname "dome"
  targetextension ""

  files { srcdir.."main.c" }
  sysincludedirs { srcdir.."include" }

  defines { 
    "DOME_VERSION=\""..version.."\"",
    "HASH=\""..hash.."\""
  }

  enablewarnings {
    "all", 
    "extra"
  }

  disablewarnings {
    "unused-parameter",
    "unused-value",
    "unused-function"
  }

  buildoptions { 
    "-pedantic",
    "`$(shell "..sdlConfig..") --cflags`"
  }

  links { "wren", "m" }

  filter "platforms:static*"
    syslibdirs { srcdir.."lib" }
    linkoptions { 
    " `$(shell "..sdlConfig..") --static-libs` "
    }
    sysincludedirs { srcdir.."include/SDL2" }
  filter "platforms:shared*"
    linkoptions { 
      "`$(shell "..sdlConfig..") --libs`",
      "-L"..libdir
    }



  filter "system:windows"
    systemversion "latest"
    -- TODO: ICON_OBJECT_FILE

  filter { "system:windows", "system:linux" }
    disablewarnings {
      "discarded-qualifiers",
      "clobbered"
    }

  filter "system:macosx"
    postbuildcommands { 
      "install_name_tool -change /usr/local/opt/sdl2/lib/libSDL2-2.0.0.dylib \\@executable_path/libSDL2-2.0.0.dylib ./dome",
      "install_name_tool -change /usr/local/lib/libSDL2-2.0.0.dylib \\@executable_path/libSDL2-2.0.0.dylib ./dome",
      "install_name_tool -add_rpath \\@executable_path/libSDL2.0.0.dylib ./dome"
    }
    disablewarnings {
      "incompatible-pointer-types-discards-qualifiers"
    }
    buildoptions { 
      "-mmacosx-version-min=10.14"
    }

  filter "configurations:Debug"
    defines { "DEBUG=1" }
    symbols "On"

  filter "configurations:Release"
    optimize "On"

  filter "platforms:32bit"
    architecture "x86"

  filter "platforms:64bit"
    architecture "x86_64"

