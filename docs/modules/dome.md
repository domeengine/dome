[< Back](.)

# dome

The `dome` module allows you to control various aspects of how DOME as an application operates.

It contains the following classes:

- [Process](#process)
- [Window](#window)

## Process

### Static Methods

#### `static exit(): Void`

#### `static exit(code: Number): Void`

Allows you to programmatically close DOME. This command behaves a little differently based on whether an error code is provided (defaults to `0`):

- If `code` is `0`, then this will immediately shutdown DOME in a graceful manner, but no other Wren code will execute after this call.
- Otherwise, the current Fiber will be aborted, the game loop will exit and DOME will shutdown.

## Window

### Static Fields

#### `static fullscreen: Boolean`

Set this to switch between Windowed and Fullscreen modes.

#### `static height: Number`

This is the height of the window/viewport, in pixels.

#### `static lockstep: Boolean`

Setting this to true will disable "catch up" game loop behaviour. This is useful for lighter games, or on systems where you experience a little stuttering at 60fps.

#### `static title: String`

This allows you to set and get the title of the DOME window.

#### `static vsync: String`

Setting this to true will make the renderer wait for VSync before presenting to the display. Changing this value is an expensive operation and so shouldn't be done frequently.

#### `static width: Number`

This is the width of the window/viewport, in pixels.

### Static Methods

#### `static resize(width: Number, height: Number): Void`

This allows you to control the size of DOME's window. The viewport will scale accordingly, but the canvas will NOT resize.
