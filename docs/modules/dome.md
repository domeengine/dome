[< Back](.)

# dome

The `dome` module allows you to control various aspects of how DOME as an application operates.

It contains the following classes:

- [Log](#log)
- [Platform](#platform)
- [Process](#process)
- [Version](#version)
- [Window](#window)


## Log

### Static Fields

#### `level: string`
The current level of logging. Can be one of: 

  0. `OFF`
  1. `FATAL`
  2. `ERROR`
  3. `WARN`
  4. `INFO`
  5. `DEBUG`.

When you set `Log.level`, the string will be converted to uppercase automatically.

The default level is `INFO`, and each level will include the levels before it; 
`ERROR` will include `FATAL` messages, for example.

#### `context: Stack<String>`

It isn't always clear where a log was generated from. To make it clearer, you can `push()`
a context label (of `string` type) onto this stack. The item at the top of the stack will
be included when a log is printed. When no longer needed, you can `pop()` the context 
off the stack.

See the [`Stack`](collections#stack) documentation for more information.

### Static Methods

#### `debug(text: String)` or `d(text: String)`

An `debug` log message is used for fine-grained diagnostics of an application's operations.
These are often verbose and frequent, which is why they are disabled by default.

#### `error(text: String)` or `e(text: String)`

An `error` log message can be used to indicate a problem with the running program but
not necessarily something unrecoverable.

#### `fatal(text: String)` or `f(text: String)`

A fatal log message indicates a catastrophic failure that cannot be recovered from.
This method will print to the console and then abort the current fiber as if `Fiber.abort` had been used.

#### `info(text: String)` or `i(text: String)`

An `info` log message is used for indicating routine operations. Useful to understand what
is happening but often high-level.

#### `warn(text: String)` or `w(text: String)`

An `warn` log message can be used to indicate something potentially problematic which
might be cause for investigation.

## Platform

Contains platform-specific utilities which may not always be supported.

### Static Fields

#### `name: String`
Returns the generic name of the operating system or platform, where possible.

#### `time: Number`
Returns the integer number of seconds since the unix epoch (1st January 1970).

#### `screenCount: Number`
Returns the integer number of screens on the system.

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
Default is `false`.

#### `static height: Number`

This is the height of the window/viewport, in pixels.

#### `static display: Number`

This is the display window is rendered on. Default is `0` which is users primary/main display. When going out of bounds (less than `0` or more than number of displays) will automatically loop back around 

#### `static integerScale: Boolean`

If set to true, the Canvas within the Window will be scaled by integer scaling factors only. This is useful for avoiding the "fat-pixel" look
due to a mismatch between the Canvas and Window aspect ratios. Default is `false`.

#### `static lockstep: Boolean`

Setting this to true will disable "catch up" game loop behaviour. This is useful for lighter games, or on systems where you experience a little stuttering at 60fps.
Default is `false`.

#### `static title: String`

This allows you to set and get the title of the DOME window.

#### `static vsync: Boolean`

Setting this to true will make the renderer wait for VSync before presenting to the display. Changing this value is an expensive operation and so shouldn't be done frequently.
Default is `true`.

#### `static width: Number`

This is the width of the window/viewport, in pixels.

#### `static color: Color`

This is the background color of the window. Note that to this _does not_ affect inside the drawing canvas: use `Canvas.cls()` to clear the canvas to some color. This field only affects the background color beyond canvas boundaries. Default is black. Cannot be transparent (transparency is ignored).

### Static Methods

#### `static resize(width: Number, height: Number): Void`

This allows you to control the size of DOME's window. The viewport will scale accordingly, but the canvas will NOT resize. DOME treats high DPI displays with a scaling factor (determined by the operating system) so the
true window size may not match your chosen dimensions.
