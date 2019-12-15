[< Back](.)

input
================

The `input` module allows you to retrieve the state of input devices such as the keyboard, mouse and game controllers.

It contains the following classes:

* [Keyboard](#keyboard)
* [Mouse](#mouse)

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

