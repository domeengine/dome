[< Back](.)

graphics
=============

The `graphics` module provides utilities for drawing to the screen.

It contains the following classes:

* [Canvas](#canvas)
* [Color](#color)
* [Drawable](#drawable)
* [Font](#font)
* [ImageData](#imagedata)

## Canvas

The `Canvas` class is the core api for graphical display.

### Fields
#### `static height: Number`
This is the height of the canvas/viewport, in pixels.
#### `static width: Number`
This is the width of the canvas/viewport, in pixels.

### Methods
#### `static circle(x: Number, y: Number, r: Number, c: Color) `
Draw a circle, centered at co-ordinates (_x_, _y_), with a radius _r_, in the color _c_.

#### `static circlefill(x: Number, y: Number, r: Number, c: Color) `
Draw a filled circle, centered at co-ordinates (_x_, _y_), with a radius _r_, in the color _c_.

#### `static cls() `
This clears the canvas fully, to black.

#### `static cls(c: Color) `
This clears the canvas fully, to the color _c_.

#### `static draw(object: Drawable, x: Number, y: Number) `
This method is syntactic sugar, to draw objects with a "draw(x: Number, y: Number)" method.

#### `static ellipse(x0: Number, y0: Number, x1: Number, y1: Number, c: Color) `
Draw an ellipse between (_x0, y0_) and (_x1, y1_) in the color _c_.

#### `static ellipsefill(x0: Number, y0: Number, x1: Number, y1: Number, c: Color) `
Draw a filled ellipse between (_x0, y0_) and (_x1, y1_) in the color _c_.

#### `static line(x0: Number, y0: Number, x1: Number, y1: Number, c: Color) `
Draw an 1px wide line between (_x0, y0_) and (_x1, y1_) in the color _c_.

#### `static print(str, x: Number, y: Number, c: Color) `
Print the text _str_ with the top-left corner at (_x, y_) in color _c_, using the currently set default font. See `Canvas.font` for more information.

#### `static print(str, x: Number, y: Number, c: Color, fontName: String) `
Print the text _str_ with the top-left corner at (_x, y_) in color _c_, in the specified font.

#### `static pset(x: Number, y: Number, c: Color) `
Set the pixel at (_x, y_) to the color _c_.
#### `static pget(x: Number, y: Number): Color `
Get the color of the pixel at (_x, y_).

#### `static rect(x: Number, y: Number, w: Number, h: Number, c: Color) `
Draw a rectangle with the top-left corner at (_x, y_), with a width of _w_ and _h_ in color _c_.

#### `static rectfill(x: Number, y: Number, w: Number, h: Number, c: Color) `
Draw a filled rectangle with the top-left corner at (_x, y_), with a width of _w_ and _h_ in color _c_.

#### `static resize(width: Number, height: Number)`
#### `static resize(width: Number, height: Number, c: Color)`
Resize the canvas to the given `width` and `height`, and reset the color of the canvas to `c`.
If `c` isn't provided, we default to black.

### Instance Field
#### `font: String`
This sets the name of the default font used for `Canvas.print(str, x, y, color)`. You can set this to `Font.default` to return to the DOME built-in font.

## Color

An instance of the `Color` class represents a single color which can be used for drawing to the `Canvas`.
DOME comes built-in with the PICO-8 palette, but you can also define and use your own colors in your games.

### Constructors

#### `construct hex(hexcode: String)`
Create a new color with the given hexcode as a string of six alpha-numeric values. Hex values can be upper or lowercase, with or without a `#`. 
#### `construct hsv(h: Number, s: Number, v: Number)`
Create a new color using the given HSV number and an alpha value of `255`. 
The `s` and `v` parameters must be between `0.0` and `1.0`. 
#### `construct hsv(h: Number, s: Number, v: Number, a: Number)`
Create a new color using the given HSV number and an alpha value of `a`, between `0 - 255`. 
The `s` and `v` parameters must be between `0.0` and `1.0`. 

#### `construct rgb(r: Number, g: Number, b: Number)`
Create a new color with the given RGB values between `0 - 255`, and an alpha value of `255`.

#### `construct rgb(r: Number, g: Number, b: Number, a: Number)`
Create a new color with the given RGBA values between `0 - 255`.

#### `construct new(r: Number, g: Number, b: Number)`
#### `construct new(r: Number, g: Number, b: Number, a: Number)`
Deprecated, aliases for `rgb` constructor.

### Instance Fields

#### `r: Number`
A value between `0 - 255` to represent the red color channel.

#### `g: Number`
A value between `0 - 255` to represent the green color channel.

#### `b: Number`
A value between `0 - 255` to represent the blue color channel.

#### `a: Number`
A value between `0 - 255` to represent the alpha transparency channel.

### Default Palette
The values for the colors in this palette can be found [here](https://www.romanzolotarev.com/pico-8-color-palette/).

 * `static black: Color`
 * `static darkblue : Color`
 * `static darkpurple: Color`
 * `static darkgreen: Color`
 * `static brown: Color`
 * `static darkgray: Color`
 * `static lightgray: Color`
 * `static white: Color`
 * `static red: Color`
 * `static orange: Color`
 * `static yellow: Color`
 * `static green: Color`
 * `static blue: Color`
 * `static indigo: Color`
 * `static pink: Color`
 * `static peach: Color`

In addition to these two values:
 * `static none: Color` - Representing clear transparency.
 * `static purple: Color` - `#8d3cff`, the DOME logo color.

## Drawable
Represents an object which can be drawn to the screen. Objects which conform to this interface can be passed to `Canvas.draw(drawable, x, y)`.
### Instance Methods
#### `draw(x: Number, y: Number): Void`
Draw the image at the given `(x, y)` position on the screen.

## Font

DOME includes a built-in fixed 8x8 pixel font, but you can also load and use fonts from TTF files stored on the file system.

### Static Methods
#### `static load(name: String, path: String, size: Number): Font`
Load the font file at `path`, rasterize it to the pixel height of `size`, and map this to the `name` for later reference. You will need to call this once for each font size, and `name` must be unique, or you will overwrite the old font.

#### `[fontName]: Font`
You can retrieve a specific font using the index operator, for example `Font["NewFont"]`.

### Instance Methods
#### `print(text: String, x: Number, y: Number, color: Color): Void`
Print the `text` on the canvas at `(x, y)`, in the given `color`.

### Instance Field
#### `antialias: Boolean`
TTF fonts can be scaled to any size, but to look good at certain sizes, you can set antialias to `true` so that some pixels are made partially transparent, to appear smoother. This is `false` by default.

## ImageData
### _extends Drawable_

This class represents the data from an image, such as a sprite or tilemap. 
DOME supports the following formats:
 * JPEG baseline & progressive (12 bpc/arithmetic not supported, same as stock IJG lib)
 * PNG 1/2/4/8/16-bit-per-channel
 * BMP non-1bpp, non-RLE

### Static Methods
#### `static loadFromFile(path: String): ImageData`
Load an image at the given `path` and cache it for use.

### Instance Fields
#### `height: Number`
#### `width: Number`

### Instance Methods
#### `draw(x: Number, y: Number): Void`
Draw the image at the given `(x, y)` position on the screen.

#### `drawArea(srcX: Number, srcY: Number, srcW: Number, srcH: Number, destX: Number, destY: Number): Void`
Draw a subsection of the image, defined by the rectangle `(srcX, srcY)` to `(srcX + srcW, srcY + srcH)`. The resulting section is placed at `(destX, destY)`.

#### `transform(parameterMap): Drawable`
This returns a `Drawable` which will perform the specified transforms, allowing for more fine-grained control over how images are drawn. You can store the returned drawable and reuse it across frames, while the image is loaded.

Options available are:

 * `srcX`, `srcY` - These specify the top-left corner of the source image region you wish to draw.
 * `srcW`, `srcH` - This is the width and height of the source image region you want to draw.
 * `scaleX`, `scaleY` - You can scale your image in the x and y axis, independant of each other. If either of these are negative, they result in a "flip" operation.
 * `angle` - Rotates the image. This is in degrees, and rounded to the nearest 90 degrees.
 * `mode`, `foreground` and `background` - By default, mode is `"RGBA"`, so your images will draw in their true colors. If you set it to `"MONO"`, any pixels which are black or have transparency will be drawn in the `background` color and all other pixels of the image will be drawn in the `foreground` color. Both colors must be `Color` objects, and default to `Color.black` and `Color.white`, respectively.

Transforms are applied as follows: Crop to the region, then rotate, then scale/flip.

Here is an example:
```wren
spriteSheet.transform({
  "srcX": 8, "srcY": 8,
  "srcW": 8, "srcH": 8,
  "scaleX": 2,
  "scaleY": -2,
  "angle": 90
}).draw(x, y)
```
The code snippet above:
 * crops an 8x8 tile from a spritesheet, starting from (8, 8) in it's image data
 * It then rotates it 90 degrees clockwise
 * Finally, it scales the tile up by 2 in both the X and Y direction, but it flips the tile vertically.



