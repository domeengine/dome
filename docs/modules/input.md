[< Back](.)

input
================

The `input` module allows you to retrieve the state of input devices such as the keyboard, mouse and game controllers.

It contains the following classes:

* [DigitalInput](#digitalinput)
* [Clipboard](#clipboard)
* [InputGroup](#inputgroup)
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


## InputGroup

You can use an `InputGroup` to check the state of multiple `DigitalInput` objects simultaneously. It also includes some features for checking if the input is being held.
This is useful for implementing more complex control schemes where input/key remapping is required.

### Constructor

#### `new(inputs: DigitalInput | Sequence<DigitalInput>): InputGroup`
Given either a single input, or a sequence of multiple inputs, it creates an `InputGroup` object.

### Instance Fields

#### `repeating: Boolean`
When set to `true`, this input is allowed to fire repeatedly.

#### `frequency: Number`
The number of ticks an input needs to be held for before the group considers it a repeat.

#### `threshold: Number`
The `InputGroup` won't start checking for repeats until after `threshold` ticks have occurred.

#### `down: Boolean`
Returns `true` if any of the group inputs are `down`.
#### `justPressed: Boolean`
Returns `true` if any of the group inputs are `justPressed`.
#### `firing: Boolean`
Returns `true` when an input has just ben pressed. If `repeating` is set and the input has been 
held down longer than `threshold` ticks, then `firing` will be true every `frequency` ticks.

### Instance Methods

#### `reset(): Void`
Resets the input state for the entire group, as if the input was just 
set to no longer be down. This is useful in certain scenarios where an 
input's state may be tested repeatedly, but before the user has had a 
chance to release the input.


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
When `handleText` is `true`, this method returns text entered since the previous tick. You should store this value if you need it to persist across ticks.

### Static Methods

#### `static [name]: DigitalInput`
This returns a digital input of a valid name. See `Keyboard.isKeyDown` for a list of valid names.


It contains the following classes:

* [DigitalInput](#digitalinput)
* [Clipboard](#clipboard)
* [InputGroup](#inputgroup)
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


## InputGroup

You can use an `InputGroup` to check the state of multiple `DigitalInput` objects simultaneously. It also includes some features for checking if the input is being held.
This is useful for implementing more complex control schemes where input/key remapping is required.

### Constructor

### Instance Methods


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
When `handleText` is `true`, this method returns text entered since the previous tick. You should store this value if you need it to persist across ticks.

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
The total distance the mouse wheel has scrolled horizontally in the past tick. Left is negative and right is positive.

#### `static scrollY: Number`
The total distance the mouse wheel has scrolled vertically in the past tick. Left is negative and down is positive.

#### `static x: Number`
The x position relative to the Canvas. This accounts for the window being resized and the viewport moving. If `Mouse.relative` is set, this will be the relative change of the mouse x position since the previous tick.

#### `static y: Number`
The y position relative to the Canvas. This accounts for the window being resized and the viewport moving. If `Mouse.relative` is set, this will be the relative change of the mouse y position since the previous tick.

### Static Methods

