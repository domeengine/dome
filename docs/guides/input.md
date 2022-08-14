[< Back](..)

How to read input?
==========================

DOME allows your projects to handle input from Keyboard, Mouse and SDL2-supported GamePads.
You can check on the state of a particular input during each update tick of the game loop, and
respond accordingly.

To get started, import the [`input`](/modules/input) module.

## Mouse and Keyboard

Mouse and Keyboard are read in similar ways. There's a static [`Mouse`](/modules/inpuP#mouse) and [`Keyboard`](/modules/input#keyboard) class, which
can be used to get the state of particular buttons.

For example:

```
import "input" for Mouse, Keyboard

if (Mouse["left"].down) {
  // do something
}
if (Keyboard["a"].down) {
  // do something else
}
```

More information about button state can be found in the [`DigitalInput`](/modules/input#digitalinput) documentation.

### Mouse Position

You can also get the position of the Mouse using the `x`, `y`, and `position` fields of the `Mouse` class.
```
import "input" for Mouse

var pos = Mouse.position
var x = Mouse.x
var y = Mouse.y
System.print(pos)
System.print("%(x), %(y)")
```


## GamePads

DOME assumes that a computer only has one mouse and keyboard attached to the system, but
it allows for any number of game pads to be connected. Unfortunately, this means that you need to 
handle multiple game pads, and detect which is the one to receive input from.

You can get a list of all active game pads using `GamePad.all`, and then loop through them to
pick the right one for your needs. (You could test if a specific button is pressed, for example.)

If you don't care too much about which GamePad is used, `GamePad.next` will return a single gamepad which is attached to the system. You should cache the result.

Now that you've acquired a specific [`GamePad`](/modules/input#gamepad), you can retrieve input from it.

### Buttons

GamePads usually support some set of standard buttons, listed in the [documentation](/modules/input#isbuttonpressedbutton-string-boolean). These can be accessed in the same way as the Mouse and Keyboard:

```
import "input" for GamePad
var pad = GamePad.next
if (pad != null) {
  System.print(pad["start"])
  System.print(pad["leftstick"])
}
```

### Analog Sticks

You can get the position of analog sticks (`"left"` and `"right"`) using `.getAnalogStick(_)`. Sticks report their position as values between -1.0 and 1.0.

```
import "input" for GamePad
var pad = GamePad.next
if (pad != null) {
  System.print(pad.getAnalogStick("left"))
}
```

### Triggers
Similar to the analog sticks, you can retrieve the analog position of triggers using `.getTrigger(_)`. They are represented by a value between 0.0 - 1.0.
```
import "input" for GamePad
var pad = GamePad.next
if (pad != null) {
  System.print(pad.getTrigger("left"))
}
```





