[< Back](..)

How to configure DOME
===================

Now that you have DOME [installed](/installation) you can use it to create whatever application you wish.
It comes with a number of settings pre-configured, but you may want to change some of these defaults,
to suit your project or for performance reasons.

This article goes over a few different things you might want to adjust.

## Setup a project

DOME begins execution from a "main.wren" file. It treats the location of that file as the "root" of the project directory. 
All module imports and asset paths are relative to it. 
It's recommended to keep your project contained within that directory 
(although nested folders are allowed) to make packaging your project for distribution easier.

## Set up your window

Most of DOME's engine settings can be accessed in the [`dome`](/modules/dome) module. This includes the `Window` which has a few useful settings to modify.

From this class, you can:

* Change the window title
* Resize the window
* Enable fullscreen mode
* Enable and disable VSync
* Change Window background color
* Control the game loop mode (enabling or disabling lockstep behaviour)

## Feature Management

DOME strives to be backwards-compatible, but it is possible that your project relies on features from a version of DOME that 
is newer than the version running the project. 

In that case, you can use the `Version` class (found in the `dome` module) to test the currently running version, and assert that you 
are at that minimum level.

## Error Handling

By default, DOME displays syntax errors and uncaught exceptions in a couple places for easy access:
 
 * An OS dialog is displayed, containing the encountered error.
 * The message is printed out to a `DOME-log.out` file, placed in the same location as your `main.wren` file.
 * If possible, it also prints to `stdout`, but this is only visible on a terminal.

For some, the OS dialog is quite obnoxious, so it can be disabled setting `Process.errorDialog` to `false`.




