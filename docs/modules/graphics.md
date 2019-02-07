Graphics
=============

The graphics module provides utilities for drawing to the screen.

It contains the following classes:

* [Canvas](#canvas)
* [Color](#color)
* [Point](#point)
* [ImageData](#imagedata)

## Canvas

The `Canvas` class is the core api for graphical display.

### Fields
#### `static width`
#### `static height` 

### Methods
#### `static pset(x, y, c) `
#### `static line(x0, y0, x1, y1, c) `
#### `static ellipse(x0, y0, x1, y1, c) `
#### `static ellipsefill(x0, y0, x1, y1, c) `
#### `static rect(x, y, w, h, c) `
#### `static rectfill(x, y, w, h, c) `
#### `static circle(x, y, r, c) `
#### `static circlefill(x, y, r, c) `
#### `static print(str, x, y, c) `
#### `static cls() `
#### `static cls(c) `
#### `static draw(object, x, y) `
