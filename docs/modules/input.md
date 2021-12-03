[< Back](.)

input
================

The `input` module allows you to retrieve the state of input devices such as the keyboard, mouse and game controllers.

It contains the following classes:

* [DigitalInput](#digitalinput)
* [Clipboard](#clipboard)
* [Keyboard](#keyboard)
* [Mouse](#mouse)
* [GamePad](#gamepad)

## DigitalInput

An instance of `DigitalInput` represents an input such as a keyboard key, mouse button or controller button, which can be either "pressed" or "unpressed".

### Instance Methods
#### `repeat(): Void`
Causes the current state of the input to be repeated, which can affect the `justPressed` property. This is useful in certain scenarios where an input's state may be tested repeatedly, but before the user has had a chance to release the input.

#### `reset(): Void`
Resets the input state, as if the input was just set to no longer be down. This is useful in certain scenarios where an input's state may be tested repeatedly, but before the user has had a chance to release the input.

### Instance Fields

#### `static down: Boolean`
This is true if the digital input is "pressed" or otherwise engaged.

#### `static justPressed: Boolean`
Returns true if the input was "pressed" down on this tick.

#### `static previous: Boolean`
This gives you the value of "down" on the previous tick, since input was last processed. (Depending on game loop lag, input may be processed once for multiple update ticks.)

#### `static repeats: Number`
This counts the number of ticks that an input has been engaged for. If the input isn't engaged, this should be zero.


## Clipboard

This class gives easy access to your operating system's clipboard for copy/paste operations.

### Static Fields

#### `static content: String`
Returns the current content of the clipboard. Set this field to put a new value into the user's clipboard.


## Keyboard

### Static Fields

#### `static allPressed: Map<string, DigitalInput>`
This returns a map containing the key names and corresponding `DigitalInput` objects, for all keys which are currently "down".

#### `static compositionText: String`
When `handleText` is true, this method returns the text of the currently selected composition received from the user's IME, if they are using one. Otherwise, this value is `null`.

#### `static compositionRange: Range`
When `handleText` is true, this provides a range object indicating which portion of the compositionText is being modified by the IME.

#### `static handleText: Boolean`
Indicates whether the text input features of DOME are enabled or not.

#### `static text: String`
When `handleText` is `true`, this method returns text entered since the previous frame. You should store this value if you need it to persist across frames.

### Static Methods

#### `static [name]: DigitalInput`
This returns a digital input of a valid name. See `Keyboard.isKeyDown` for a list of valid names.

#### `static isButtonPressed(key: String): Boolean`
#### `static isKeyDown(key: String): Boolean`
Returns true if the named key is pressed. The key uses the SDL key name, which can be referenced [here](https://wiki.libsdl.org/SDL_Keycode).

#### `textRegion(x: Number, y: Number, width: Number, height: Number): void`
Use this to hint to the OS at where the user is expecting entered text to be displayed. The OS may use this to display the IME in a suitable location.
This must be set while `handleText` is true, or the effect may be inconsistent.

## Mouse

### Static Fields

#### `static allPressed: Map<string, DigitalInput>`
This returns a map containing the key names and corresponding `DigitalInput` objects, for all keys which are currently "down".

#### `static cursor: String`
Gets or sets the system cursor to the name provided.
Available cursor names are:
* `arrow`
* `ibeam`
* `wait`
* `crosshair`
* `waitarrow`
* `sizenwse`
* `sizenesw`
* `sizewe`
* `sizens`
* `sizeall`
* `no`
* `hand`

Mac OS X will set the system cursor to `arrow` if `wait` or `waitarrow` is set.
Mac OS X will set the system cursor to a closed hand if `sizenwse`, `sizenesw` or `sizeall` is set.

#### `static hidden: Boolean`
Controls whether the mouse cursor is shown or hidden. You can set and read from this field.

#### `static pos: Vector`
#### `static position: Vector`
Returns a vector of _(Mouse.x, Mouse.y)_ for convenience.

#### `static relative: Boolean`
If set to true, the mouse is placed into relative mode. In this mode, the mouse will be fixed to the center of the screen. You can set and read from this field. This changes the behaviour of the `x` and `y` fields.

#### `static scroll: Vector`
Returns a vector of _(scrollX, scrollY)_, for convenience.

#### `static scrollX: Number`
The total distance the mouse wheel has scrolled horizontally in the past frame. Left is negative and right is positive.

#### `static scrollY: Number`
The total distance the mouse wheel has scrolled vertically in the past frame. Left is negative and down is positive.

#### `static x: Number`
The x position relative to the Canvas. This accounts for the window being resized and the viewport moving. If `Mouse.relative` is set, this will be the relative change of the mouse x position since the previous tick.

#### `static y: Number`
The y position relative to the Canvas. This accounts for the window being resized and the viewport moving. If `Mouse.relative` is set, this will be the relative change of the mouse y position since the previous tick.

### Static Methods

#### `static [name]: DigitalInput`
This returns a digital input of a valid name. See `Mouse.isButtonPressed` for a list of valid names.

#### `static isButtonPressed(name: String/Number): Boolean`
Returns true if the named mouse button is pressed. 
You can use an index from 1-5 (button 0 is invalid) or a lowercase name:
* `left`
* `middle`
* `right`
* `x1`
* `x2`

## GamePad

You can use a game pad as input for your games. DOME expects a game pad similar to those used by popular games consoles, with a D-Pad, face buttons, triggers and analog sticks.

### Static Fields

#### `all: List<GamePad>`
Returns a list of GamePad objects representing all attached gamepads.

#### `next: GamePad`
Returns a GamePad representing an arbitrary attached gamepad. This isn't guarenteed to return the same object every time, so you should cache it.

### Static Methods

#### `[id]: GamePad`
This will return an object representing a GamePad. You can then read the state of that gamepad using the instance methods below.

If no gamepads are attached, you will receive a "dummy" object which will report null or empty values.

### Instance Fields

#### `allPressed: Map<string, DigitalInput>`
This returns a map containing the key names and corresponding `DigitalInput` objects, for all keys which are currently "down".

#### `attached: Boolean`
This returns true if the gamepad is still attached to the system.
#### `id: Number`
Returns the instance id for this gamepad, which can be used to fetch it using `GamePad[id]`.
#### `name: String`
If the gamepad is attached, this returns the SDL internal name for that device.

### Instance Methods

#### `[name]: DigitalInput`
This returns a digital input of a valid name. See `GamePad.isButtonPressed` for a list of valid names.

#### `isButtonPressed(button: String): Boolean`
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
Gets the current state of the specified analog stick as a Vector, with `x` and `y` values, which are normalised as values between -1.0 and 1.0.
Valid sides are `left` and `right`.

#### `rumble(strength: Number, duration: Number): Void`
If the gamepad is able to vibrate, it will vibrate with a `strength`, clamped between `0.0` and `1.0`, for a `duration` of milliseconds. Rumble and haptic feedback is dependant on platform drivers.
