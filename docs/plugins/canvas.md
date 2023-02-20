[< Back](.)

Canvas
===============

This set of APIs allows you to modify what is displayed on the main canvas.
You can exploit this to allow for more efficient graphical rendering techniques.

* [Acquisition](#acquistion)
* Enums
  - [enum: DOME_DrawMode](#enum-dome_drawmode)
* Struct
  - [struct: DOME_Color](#method-dome_color)
* Methods
  - [method: draw](#method-draw)
  - [method: getWidth](#method-getwidth)
  - [method: getHeight](#method-getHeight)
  - [method: line](#method-line)
  - [method: pget](#method-pget)
  - [method: pset](#method-pset)
  - [method: rect](#method-rect)
  - [method: rectfill](#method-rectfill)
  - [method: unsafePset](#method-unsafepset)

## Acquisition

```c
CANVAS_API_v0* canvas = (CANVAS_API_v0*)DOME_getAPI(API_CANVAS, CANVAS_API_VERSION);
```

## Enums
### enum: DOME_DrawMode

Some methods in this API allow you to enable or disable alpha-blending 
for performance gains.

```c
enum DOME_DrawMode {
  DOME_DRAWMODE_BLEND
}
```

## Struct
### struct: DOME_Color

The DOME_Bitmap type contains the following fields:

| Field  | Type        | Purpose                                          |
| ----------------------------------------------------------------------- |
|    a   | uint8_t     | This is the color's alpha channel, from 0 - 255. |
|    r   | uint8_t     | This is the color's red channel, from 0 - 255.   |
|    g   | uint8_t     | This is the color's green channel, from 0 - 255. |
|    b   | uint8_t     | This is the color's blue channel, from 0 - 255.  |

This type is also a union. You can get all the fields simultaneously as a 
32-bit integer, `value`, arranged in the layout `0xAABBGGRR`.

## Methods
### method: draw
```c
void draw(DOME_Context ctx, DOME_Bitmap* bitmap, int32_t x, int32_t y, DOME_DRAWMODE mode)
```
Draws the `bitmap` to the canvas at `(x, y)`. If the `mode` is set, alpha-blending
will be applied. This will ignore the canvas draw context (offset, clipping region, etc).

### method: getWidth
```c
uint32_t getWidth(DOME_Context ctx)
```
Returns the width of the canvas, in pixels.

### method: getHeight
```c
uint32_t getHeight(DOME_Context ctx)
```
Returns the height of the canvas, in pixels.

### method: line
```c
void line(DOME_Context ctx, int64_t x0, int64_t y0, int64_t x1, int64_t y1, DOME_Color color);
```
Draws a one pixel wide line between `(x0, y0)` and `(x1, y1)`, in the chosen `color`.

### method: pget
```c
DOME_Color pget(DOME_Context ctx, uint32_t x, uint32_t y)
```
Returns the color of the pixel located at `(x, y)` in the canvas.

### method: pset
```c
void pset(DOME_Context ctx, uint32_t x, uint32_t y, DOME_Color color)
```
Sets the color of the pixel located at `(x, y)` in the canvas to `color`.

### method: rect
```c
void rect(DOME_Context ctx, int64_t x, int64_t y, int64_t width, int64_t height, DOME_Color color);
```

Draws a rectangle (edges only) with size `(width, height)`, with it's top-left corner at `(x, y)`.

### method: rectfill
```c
void rectfill(DOME_Context ctx, int64_t x, int64_t y, int64_t width, int64_t height, DOME_Color color);
```

Draws a filled rectangle with size `(width, height)`, with it's top-left corner at `(x, y)`.

### method: unsafePset
```c
void unsafePset(DOME_Context ctx, uint32_t x, uint32_t y, DOME_Color color)
```
Sets the color of the pixel located at `(x, y)` in the canvas to `color`.
This function is provided for performance-sensitive applications.
It does not do range checks. If you attempt to set a pixel outside the canvas,
you risk crashing DOME.
