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
#### `static height` 
This is the height of the canvas/viewport, in pixels.
#### `static width`
This is the width of the canvas/viewport, in pixels.

### Methods
#### `static circle(x, y, r, c: Color) `
Draw a circle, centered at co-ordinates (_x_, _y_), with a radius _r_, in the color _c_.

#### `static circlefill(x, y, r, c: Color) `
Draw a filled circle, centered at co-ordinates (_x_, _y_), with a radius _r_, in the color _c_.

#### `static cls() `
This clears the canvas fully, to black.

#### `static cls(c: Color) `
This clears the canvas fully, to the color _c_.

#### `static draw(object, x, y) `
This method is syntactic sugar, to draw objects with a "draw(x, y)" method.

#### `static ellipse(x0, y0, x1, y1, c: Color) `
Draw an ellipse between (_x0, y0_) and (_x1, y1_) in the color _c_.

#### `static ellipsefill(x0, y0, x1, y1, c: Color) `
Draw a filled ellipse between (_x0, y0_) and (_x1, y1_) in the color _c_.

#### `static line(x0, y0, x1, y1, c: Color) `
Draw an 1px wide line between (_x0, y0_) and (_x1, y1_) in the color _c_.

#### `static print(str, x, y, c: Color) `
Print the text _str_ with the top-left corner at (_x, y_) in color _c_.

#### `static pset(x, y, c: Color) `
Set the pixel at (_x, y_) to the color _c_.

#### `static rect(x, y, w, h, c: Color) `
Draw a rectangle with the top-left corner at (_x, y_), with a width of _w_ and _h_ in color _c_.

#### `static rectfill(x, y, w, h, c: Color) `
Draw a filled rectangle with the top-left corner at (_x, y_), with a width of _w_ and _h_ in color _c_.

## Color

An instance of the `Color` class represents a single color which can be used for drawing.

#### `construct new(r, g, b)`
#### `construct new(r, g, b, a)`
#### `static rgb(r, g, b, a)`
#### `static black`
#### `static blue`
#### `static cyan`
#### `static darkgray`
#### `static green`
#### `static lightgray`
#### `static orange`
#### `static red`
#### `static white`

## Point

The `Point` class is a 2-dimensional vector

`TODO`

## ImageData

This class represents the data from an image, such as a sprite or tilemap.

### Static Methods
#### `static loadFromFile(path)`

### Instance Fields
#### `foreign height`
#### `foreign width`

### Instance Methods
#### `construct fromFile(path)`
#### `foreign draw(x, y)`
#### `foreign drawArea(srcX, srcY, srcW, srcH, destX, destY)`
