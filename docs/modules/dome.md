[< Back](.)

dome
================

The `dome` module allows you to control various aspects of how DOME as an application operates.

It contains the following classes:

* [Process](#process)
* [Window](#window)

## Process

### Static Methods

#### `static exit(): Void`
#### `static exit(code: Number): Void`
Allows you to programmatically close DOME. This command behaves a little differently based on whether an error code is provided (defaults to `0`):
 * If `code` is `0`, then this will immediately shutdown DOME in a graceful manner, but no other Wren code will execute after this call.
 * Otherwise, the current Fiber will be aborted, the game loop will exit and DOME will shutdown.

## Window

### Static Fields

#### `static title: String`
This allows you to set and get the title of the DOME window.

### Static Methods

#### `static resize(width: Number, height: Number): Void`
This allows you to control the size of DOME's window. The viewport will scale accordingly, but the canvas will NOT resize.
