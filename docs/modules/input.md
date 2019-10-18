[< Back](.)

input
================

The `input` module allows you to retrieve the state of input devices such as the keyboard, mouse and game controllers.

It contains the following classes:

* [Keyboard](#keyboard)

## Keyboard

### Static Methods

#### `static isKeyDown(key: String): Boolean`
Returns true if the named key is pressed. The key uses the SDL key name, which can be referenced [here](https://wiki.libsdl.org/SDL_Keycode).
