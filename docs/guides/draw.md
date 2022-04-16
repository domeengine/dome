[< Back](..)

How to draw to the screen?
==========================

An important part of most applications is the ability to draw on screen.
DOME lets you draw primitive shapes (lines, rectangles, circles and ovals) as 
well as sprites loaded from image files.

Make sure to draw in DOME during the `draw()` portion of DOME's game-loop.
The reason for this is discussed further in the [Game Loop guide]().

You can access most of the graphical functions through the [`Canvas`]() class
in the [`graphics`]() module.

## Primitives

The `Canvas` class contains functions for drawing the different shapes. There's an outline and "filled" version:

 * `Canvas.line` - Draw a line between two points. There's an optional thickness option.
 * `Canvas.rect`, `Canvas.rectfill` - Draw a rectangle. You need to give it a top-left corner point, and a size.
 * `Canvas.circle`, `Canvas.circlefill` - Draw a circle. Provide it a center and a radius.
 * `Canvas.ellipse`, `Canvas.ellipsefill` - Draw an ellipse. It requires a top-left and bottom-right corner, and draws between the two.

These also require a [`Color`]() value. There's a collection of default colors built into DOME, but you can also specify 
custom colors using `Color.rgb`, `Color.hex` and `Color.hsv`.

## Images

You'll need the `ImageData` class from the `graphics` module for these.

DOME supports the following image formats:

* JPEG baseline & progressive (12 bpc/arithmetic not supported, same as stock IJG lib)
* PNG 1/2/4/8/16-bit-per-channel
* BMP non-1bpp, non-RLE

Here's an example, loading an image and drawing it to the Canvas:

```javascript
var image = ImageData.loadFromFile("path/to/file.format")
image.draw(6, 7)
```

DOME also supports scaling and rotation, as well as drawing sub-regions, and that can be achieved using 
the [`ImageData.transform`]() method.




