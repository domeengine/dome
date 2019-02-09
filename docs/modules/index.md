Modules
============

DOME includes a number of modules to interact with the user through graphics and audio, access their inputs and hard drive. It also has a few useful utilities for mathematical operations.

The modules you can import are here:

* [Audio](audio)
* [Graphics](graphics)
* [Input](input)
* IO

For example, the `Graphics` module can be imported to access the `Canvas` and `Color` classes, like this:

```wren
import "graphics" for Canvas, Color

...

Canvas.cls()
Canvas.print("Hello world", 20, 20, Color.white)
```



