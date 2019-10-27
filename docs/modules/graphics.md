[< Back](.)

graphics
=============

The `graphics` module provides utilities for drawing to the screen.

It contains the following classes:

* [Canvas](#canvas)
* [Color](#color)
* [Point](#point)
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

#### `static draw(object, x: Number, y: Number) `
This method is syntactic sugar, to draw objects with a "draw(x: Number, y: Number)" method.

#### `static ellipse(x: Number0, y: Number0, x1, y: Number1, c: Color) `
Draw an ellipse between (_x0, y0_) and (_x1, y1_) in the color _c_.

#### `static ellipsefill(x: Number0, y: Number0, x1, y: Number1, c: Color) `
Draw a filled ellipse between (_x0, y0_) and (_x1, y1_) in the color _c_.

#### `static line(x: Number0, y: Number0, x1, y: Number1, c: Color) `
Draw an 1px wide line between (_x0, y0_) and (_x1, y1_) in the color _c_.

#### `static print(str, x: Number, y: Number, c: Color) `
Print the text _str_ with the top-left corner at (_x, y_) in color _c_.

#### `static pset(x: Number, y: Number, c: Color) `
Set the pixel at (_x, y_) to the color _c_.

#### `static rect(x: Number, y: Number, w: Number, h: Number, c: Color) `
Draw a rectangle with the top-left corner at (_x, y_), with a width of _w_ and _h_ in color _c_.

#### `static rectfill(x: Number, y: Number, w: Number, h: Number, c: Color) `
Draw a filled rectangle with the top-left corner at (_x, y_), with a width of _w_ and _h_ in color _c_.

## Color

An instance of the `Color` class represents a single color which can be used for drawing.

#### `construct new(r: Number, g: Number, b: Number)`
#### `construct new(r: Number, g: Number, b: Number, a: Number)`
#### `static rgb(r: Number, g: Number, b: Number, a: Number): Color`
#### `static black: Color`
#### `static blue: Color`
#### `static cyan: Color`
#### `static darkgray: Color`
#### `static green: Color`
#### `static lightgray: Color`
#### `static orange: Color`
#### `static red: Color`
#### `static white: Color`

## Point

The `Point` class is a 2-dimensional vector

`TODO`

## ImageData

This class represents the data from an image, such as a sprite or tilemap. 
DOME uses stb_image to load images, so it supports the same formats:
 * JPEG baseline & progressive (12 bpc/arithmetic not supported, same as stock IJG lib)
 * PNG 1/2/4/8/16-bit-per-channel
 * TGA (not sure what subset, if a subset)
 * BMP non-1bpp, non-RLE
 * PSD (composited view only, no extra channels, 8/16 bit-per-channel)
 * GIF (*comp always reports as 4-channel)
 * HDR (radiance rgbE format)
 * PIC (Softimage PIC)
 * PNM (PPM and PGM binary only)

### Static Methods
#### `static loadFromFile(path: String): ImageData`

### Instance Fields
#### `foreign height: Number`
#### `foreign width: Number`

### Instance Methods
#### `foreign draw(x: Number, y: Number)`
#### `foreign drawArea(srcX: Number, srcY: Number, srcW: Number, srcH: Number, destX: Number, destY: Number)`
