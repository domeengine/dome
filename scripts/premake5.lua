srcdir = "../src/"
libdir = "src/lib/"
moduledir = "src/modules/"
utildir = "src/util/"
sdlConfig = "which sdl2-config 1>/dev/null && echo \"sdl2-config\" || echo \""..libdir.."sdl2-config\""

workspace "DOME"
  configurations { "Release", "Debug" }
  platforms { 
    "static64", 
    "shared64", 
    "static32", 
    "shared32"
  }
  defaultplatform "shared64"
  location ".."
  basedir ".."
  startproject "DOME"

  filter "platforms:static*"
    tags { "static" }
  filter "platforms:shared*"
    tags { "shared" }
  filter "platforms:*64"
    tags { "64bit" }
  filter "platforms:*32"
    tags { "32bit" }

project "wren"
  kind "Makefile"
  language "C"
  cdialect "c99"
  targetdir "../src/lib"
  targetextension ".a"

  cleancommands {
    "{DELETE} "..libdir.."libwren.a",
    "{DELETE} "..libdir.."libwrend.a"
  }

  filter "configurations:Debug"
    targetname "libwrend"
  filter "configurations:Release"
    targetname "libwren"

  filter "tags:32bit"
    buildcommands {
      "./scripts/setup_wren.sh 32bit WREN_OPT_RANDOM=1 WREN_OPT_META=1"
    }

  filter "tags:64bit"
    buildcommands {
      "./scripts/setup_wren.sh 64bit WREN_OPT_RANDOM=1 WREN_OPT_META=1"
    }

project "modules"
  kind "Makefile"
  language "C"
  cdialect "c99"
  files { 
    utildir.."embed.c",
    moduledir.."*.wren" 
  }
  targetdir (moduledir)
  targetname "*"
  targetextension ".inc"

  buildcommands {
    "./scripts/generateEmbedModules.sh"
  }

  cleancommands {
    "{DELETE} "..moduledir.."*.inc",
    "{DELETE} "..utildir.."embed"
  }

project "dome"
  dependson { 
    "wren",
    "modules"
  }
  kind "WindowedApp"
  language "C"
  cdialect "c99"
  targetdir ".."
  targetname "dome"

  files { srcdir.."main.c" }
  sysincludedirs { srcdir.."include" }

  defines { 
    "DOME_VERSION=\"$(VERSION)\"",
    "HASH=\"$(HASH)\""
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
    "`$(SDLCONFIG) --cflags`"
  }


  makesettings [[
VERSION=`git describe --tags`
HASH=`git rev-parse --short HEAD`
SDLCONFIG=$(shell which sdl2-config 1>/dev/null && echo \"sdl2-config\" || echo \""..libdir.."sdl2-config\")
  ]]

  links { "m" }
  linkoptions {
    "-lwren"
  }

  filter "tags:static"
    syslibdirs { srcdir.."lib" }
    linkoptions { 
    " `$(SDLCONFIG) --static-libs` "
    }
    sysincludedirs { srcdir.."include/SDL2" }
  filter "tags:not static"
    linkoptions { 
      "`$(SDLCONFIG) --libs`",
      "-L"..libdir
    }



  filter "system:windows"
    systemversion "latest"
  filter { "system:windows", "tags: static" }
    linkoptions {
      "-static"
    }
  filter { "system:windows", "tags: *32" }
    targetname "dome-x32"
  filter { "system:windows", "tags: *64" }
    targetname "dome-x64"
    -- TODO: ICON_OBJECT_FILE

  filter { "system:windows", "system:linux" }
    disablewarnings {
      "discarded-qualifiers",
      "clobbered"
    }

  filter "system:macosx"
    targetextension ""
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

  filter "tags:32bit"
    architecture "x86"

  filter "tags:64bit"
    architecture "x86_64"

