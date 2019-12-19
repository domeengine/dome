[< Back](.)

input
================

The `input` module allows you to retrieve the state of input devices such as the keyboard, mouse and game controllers.

It contains the following classes:

* [Keyboard](#keyboard)
* [Mouse](#mouse)
* [GamePad](#gamepad)

## Keyboard

### Static Methods

#### `static isKeyDown(key: String): Boolean`
Returns true if the named key is pressed. The key uses the SDL key name, which can be referenced [here](https://wiki.libsdl.org/SDL_Keycode).

## Mouse

### Static Fields

#### `static x: Number`
The x position relative to the Canvas. This accounts for the window being resized and the viewport moving.

#### `static y: Number`
The y position relative to the Canvas. This accounts for the window being resized and the viewport moving.

### Static Methods

#### `static isButtonPressed(name: String/Number): Boolean`
Returns true if the named mouse button is pressed. 
You can use an index from 1-5 (button 0 is invalid) or a lowercase name:
* `left`
* `middle`
* `right`
* `X1`
* `X2`

## GamePad

You can use a game pad as input for your games. DOME expects a game pad similar to those used by popular games consoles, with a D-Pad, face buttons, triggers and analog sticks.

DOME does not support hot-plugging. A gamepad has to be plugged in before the application starts up in order to be read. If it is disconnected, the game will no longer be able to read it's values, even if it is plugged in.

### Static Methods

#### `static discover(): List<Number>`
This is called at start up and initialises all connected game pads. You can call it to fetch the total number of discovered gamepads

#### `[id]: GamePad`
This will return an object representing a GamePad. You can then read the state of that gamepad using the instance methods below.

If no gamepads are attached, you will receive a "dummy" object which will report null or empty values.

### Instance Fields
#### `attached: Boolean`
This returns true if the gamepad is still attached to the system.
#### `name: String`
If the gamepad is attached, this returns the SDL internal name for that device.

### Instance Methods

#### `isButtonPressed(key: String): Boolean`
Returns true if the named button is pressed. Valid button names are:
 * `left` - D-Pad Left
 * `right` - D-Pad Right
 * `up` - D-Pad Up
 * `down` - D-Pad Down
 * `A` - A Button
 * `B` - B Button
 * `X` - X Button
 * `Y` - Y Button
 * `back` - Back Button (Often referred to as "Select")
 * `start` - Start Button
 * `guide` - Guide Button (On an XBox controller, this is the XBox button)
 * `leftstick` - Clicking the left stick
 * `rightstick` - Clicking the right stick
 * `leftshoulder` - Left shoulder button
 * `rightshoulder` - Right shoulder button

These button names are case insensitive.

#### `getTrigger(side: String): Number`
Gets the current state of the trigger on the specified `side`, as a number between 0.0 and 1.0.
Valid sides are `left` and `right`.

#### `getAnalogStick(side: String): Vector`
Gets the current state of the specified analog stick as a Vector, with `x` and `y` values. 
Valid sides are `left` and `right`.

