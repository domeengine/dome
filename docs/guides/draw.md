[< Back](..)

How to draw to the screen?
==========================

An important part of most applications is the ability to draw on screen.
DOME lets you draw primitive shapes (lines, rectangles, circles and ovals) as 
well as sprites loaded from image files.

Make sure to draw in DOME during the `draw()` portion of DOME's game-loop.
The reason for this is discussed further in the [Game Loop guide](/guides/game-loop).

You can access most of the graphical functions through the [`Canvas`](/modules/graphics#canvas) class
in the [`graphics`](/modules/graphics) module.

## Primitives

The `Canvas` class contains functions for drawing the different shapes. There's an outline and "filled" version:

 * `Canvas.line` - Draw a line between two points. There's an optional thickness option.
 * `Canvas.rect`, `Canvas.rectfill` - Draw a rectangle. You need to give it a top-left corner point, and a size.
 * `Canvas.circle`, `Canvas.circlefill` - Draw a circle. Provide it a center and a radius.
 * `Canvas.ellipse`, `Canvas.ellipsefill` - Draw an ellipse. It requires a top-left and bottom-right corner, and draws between the two.

These also require a [`Color`](/modules/graphics#color) value. There's a collection of default colors built into DOME, but you can also specify 
custom colors using `Color.rgb`, `Color.hex` and `Color.hsv`.

## Text

Most applications will require some kind of text to be displayed on screen to convey things
to the user. DOME comes with a built-in pixel font (8x8 pixels), as well as support for TTF-based fonts.

In order to print text on screen, you can use `Canvas.print(text, x, y, color)`. This uses the built-in font by 
default, but you can change this.

Here's how you use a font:

```javascript
import "graphics" for Font, Color

// Default font
Canvas.print("Defaults Words", 0, 0, Color.green)

var fontPointSize = 16
var myFont = Font.load("uniqueName", "path/to/file.format", fontPointSize)

// You can print in two ways. Canvas:
Canvas.print("Words", 0, 8, Color.green, "uniqueName")
// or the Font class:
Font["uniqueName"].print("Words", 0, 8, Color.green)
```

DOME also lets you change the default font used for `Canvas.print`, by setting `Canvas.font` to the font's unique name.

```javascript
import "graphics" for Font, Color

var fontPointSize = 16
var myFont = Font.load("uniqueName", "path/to/file.format", fontPointSize)

Canvas.font = "uniqueName"
// Print using the font
Canvas.print("Words", 0, 8, Color.green)

Canvas.font = Font.default
Canvas.print("Default words", 0, 0, Color.green)
```

Look at the [`Font`](/modules/graphics#font) class for more information.

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
the [`ImageData.transform`](/modules/graphics#transformparametermap-drawable) method.




