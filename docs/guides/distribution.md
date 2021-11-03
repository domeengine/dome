[< Back](..)

Distributing your games
===================

DOME is designed to be cross-platform, and so the same Wren code and assets should work across Windows, Mac and Linux.

## Basic Packaging
The easiest way to share a game you've made is to place a DOME binary, your source code and game assets into a single zip file. Do this once for each platform you wish to support, and share those zip files with your users.
If you are providing your own DOME binary, rather than a pre-compiled one, you will need to have access to the shared SDL2 library.

## nest - Easy bundling

For easy distribution, you can package all of your application code and resources into a single `.egg` file using the `nest` tool built into DOME. DOME automatically plays any file named `game.egg` in the current working directory.

If you use a `.egg` file, DOME expects your game to start from a `main.wren` in the base of the bundle, as its entry point.

Navigate to your main game directory, before running the following on the commandline:

```
> dome nest [files | directories]
```

This will bundle all the files and directories into a file named `game.egg`.

### Fused Mode

Depending on your needs, you might want to only distribute your application to your users as a single file. This is possible using DOME's "fuse" mode. Once you have a `.egg` file as described in the previous section, you can embed it inside a DOME executable using the "fuse" tool. Run DOME from the commandline like this:

```
> dome fuse game.egg [destination file]
```

This creates a standalone executable which requires no other files to run. If you set a destination file, the resulting binary will be placed there. Otherwise, it'll be placed in the current working directory, as a file named `game` (or `game.exe` on Windows). This will only produce a binary for the current platform. You'll need to do this on each platform you want to distribute to.

## Platform-specific Distribution Notes

This section discusses the needs of various platforms when distributing games with DOME.

### Windows

On Windows, DOME comes compiled with all it's dependancies, so you just need to provide your game files.

### Mac OS X

On Mac, you can create an application bundle by arranging your code and assets into the following directory/file layout:

`<Game>` is a placeholder and should be replaced by the name of your application. It must be named consistently in the bundle layout, as well as in the Info.plist file.

```
+-- <Game>.app
    +-- Contents
        +-- Info.plist
        +-- MacOS
            +-- dome
            +-- libSDL2.dylib
            +-- <Game> (This is a small runscript)
        +-- Resources
            +-- game.egg
            +-- icon.icns
```

You only need to provide libSDL2.dylib if you are not using a statically linked version of `dome`. (If you are using an official DOME binary, you don't need to worry.)

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
  <string> <Game> </string>
  <key>CFBundleIconFile</key>
  <string>icon</string>
  <key>NSHighResolutionCapable</key>
  <true/>
</dict>
</plist>
```

The .app bundle, runscript and the `CFBundleExecutable` must all be named the same.

Doing this results in a self contained and easy to distribute application bundle.

### Linux

To run your game on linux, make sure SDL2 is installed, then run the `dome` executable with a `game.egg` file in the same directory.

### Web

DOME has an experimental web engine, which you can use to play your game in a browser. 
To do this, you need to host the `dome.html` file and your `game.egg` file in the same directory on a server.

You may find that the performance of your game suffers when running in the browser, in which case it may not be suitable for playing in browsers.
DOME's web engine does not currently support playing at full screen.

