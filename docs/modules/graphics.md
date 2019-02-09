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
#### `static width`

### Methods
#### `static circle(x, y, r, c) `
#### `static circlefill(x, y, r, c) `
#### `static cls() `
#### `static cls(c) `
#### `static draw(object, x, y) `
#### `static ellipse(x0, y0, x1, y1, c) `
#### `static ellipsefill(x0, y0, x1, y1, c) `
#### `static line(x0, y0, x1, y1, c) `
#### `static print(str, x, y, c) `
#### `static pset(x, y, c) `
#### `static rect(x, y, w, h, c) `
#### `static rectfill(x, y, w, h, c) `

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
