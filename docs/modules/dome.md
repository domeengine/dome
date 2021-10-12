[< Back](.)

# dome

The `dome` module allows you to control various aspects of how DOME as an application operates.

It contains the following classes:

- [Platform](#platform)
- [Process](#process)
- [Version](#version)
- [Window](#window)

## Platform

Contains platform-specific utilities which may not always be supported.

### Static Fields

#### `name: String`
Returns the generic name of the operating system or platform, where possible.

#### `time: Number`
Returns the integer number of seconds since the unix epoch (1st January 1970).

## Process

### Static Fields

#### `static errorDialog : Boolean`

Allows you to enable/disable error message boxes and is enabled by default. You can both get and set this value.

#### `static args: String[]`
Returns a string list of all non-option command line arguments used to invoke DOME's execution. The first two elements of the returned list will be:

1. the path and name of DOME's invokation.
2. The entry path, combined with the evaluated basepath.

### Static Methods

#### `static exit(): Void`

#### `static exit(code: Number): Void`

Allows you to programmatically close DOME. This command behaves a little differently based on whether an error code is provided (defaults to `0`):

- If `code` is `0`, then this will immediately shutdown DOME in a graceful manner, but no other Wren code will execute after this call.
- Otherwise, the current Fiber will be aborted, the game loop will exit and DOME will shutdown.

## Version
This class provides information about the version of DOME which is currently running. You can use this to check that all the features you require are supported.
DOME uses semantic versioning, split into a major.minor.patch breakdown.

### Static Fields
#### `static major: Number`
The major component of the version number.
#### `static minor: Number`
The minor component of the version number.
#### `static patch: Number`
The patch component of the version number.
#### `static toString: String`
A string containing the complete semantic version of the DOME instance running.

#### `static toList: List<Number>`
A list of the version components, in `[major, minor, patch]` order.

### Static Methods
#### `static atLeast(version: String): boolean`
This takes a version as a string of the form `x.y.z`, and returns true if the current version of DOME is at least that of the version specified.


## Window

### Static Fields

#### `static fps: Number`

This is the current frames per second number.

#### `static fullscreen: Boolean`

Set this to switch between Windowed and Fullscreen modes.

#### `static height: Number`

This is the height of the window/viewport, in pixels.

#### `static lockstep: Boolean`

Setting this to true will disable "catch up" game loop behaviour. This is useful for lighter games, or on systems where you experience a little stuttering at 60fps.

#### `static title: String`

This allows you to set and get the title of the DOME window.

#### `static vsync: Boolean`

Setting this to true will make the renderer wait for VSync before presenting to the display. Changing this value is an expensive operation and so shouldn't be done frequently.

#### `static width: Number`

This is the width of the window/viewport, in pixels.

#### `static color: Color`

This is the background color of the window. Note that to this _does not_ affect inside the drawing canvas: use `Canvas.cls()` to clear the canvas to some color. This field only affects the background color beyond canvas boundaries. Default is black. Cannot be transparent (transparency is ignored).

### Static Methods

#### `static resize(width: Number, height: Number): Void`

This allows you to control the size of DOME's window. The viewport will scale accordingly, but the canvas will NOT resize.
