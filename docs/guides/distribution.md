[< Back](..)

Distributing your games
===================

DOME is designed to be cross-platform, and so the same Wren game files and assets should work across Windows, Mac and Linux. On Mac and Linux, the SDL2 shared library will need to be provided.

# NEST - Easy packaging

For easy distribution, you can package all of your games resources into a single `.egg` file using a tool called [NEST](https://github.com/avivbeeri/nest). DOME automatically plays any file named `game.egg` in the current working directory.

DOME expects your game.wren to use a `main.wren` in the base directory as it's entry point.

Install NEST, and then navigate to your main game directory, before running the following:

```
> nest -z -o game.egg -- [files | directories]
```

## Packaging and Dependancies

### Windows

On Windows, DOME comes compiled with all it's dependancies, so you just need to provide your game files.

### Mac OS X

On Mac, the SDL2 library needs to be provided. This can either be globally installed, or you can package it as part of an application bundle, using the following layout:

```
+-- Game.app
    +-- Contents
        +-- Info.plist
        +-- MacOS
            +-- dome
            +-- libSDL2.dylib
            +-- Game (This is a small runscript)
        +-- Resources
            +-- game.egg
            +-- icon.icns
```

The runscript is very simple and looks like this:
```bash
#!/bin/bash
cd "${0%/*}"
./dome
```

Finally, the Info.plist at minimum needs to look like this:
```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleExecutable</key>
  <string>Game</string>
  <key>CFBundleIconFile</key>
  <string>icon</string>
  <key>NSHighResolutionCapable</key>
  <true/>
</dict>
</plist>
```

The .app bundle, runscript and the `CFBundleExecutable` must all be named the same.

Doing this results in a self contained and easy to distribute application bundle.

### Linux

To run your game on linux, make sure SDL2 is installed, then run the dome executable with a `game.egg` file in the same directory.





