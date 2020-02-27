/**
  @Module graphics
  The graphics module provides all the system functions required for drawing to the screen.
*/
import "vector" for Point, Vec, Vector
import "image" for Drawable, ImageData

/**
    @Class Canvas
      This class provides static methods for drawing primitives and images.
*/
class Canvas {
  foreign static f_resize(width, height, color)
  static resize(width, height) { resize(width, height, Color.black) }
  static resize(width, height, c) {
    if (width < 0) {
      Fiber.abort("Window can't have a negative width")
    }
    if (height < 0) {
      Fiber.abort("Window can't have a negative height")
    }

    if (width > 4096) {
      Fiber.abort("Window can't be wider than 4096")
    }
    if (height > 2160) {
      Fiber.abort("Window can't be wider than 2160")
    }
    if (c is Color) {
      f_resize(width, height, c.rgb)
    } else {
      f_resize(width, height, c)
    }
  }
  foreign static f_pset(x, y, c)
  foreign static f_line(x1, y1, x2, y2, c)
  foreign static f_rectfill(x, y, w, h, c)
  foreign static f_rect(x, y, w, h, c)
  foreign static f_print(str, x, y, c)
  foreign static f_circle(x, y, r, c)
  foreign static f_circlefill(x, y, r, c)
  foreign static f_ellipse(x1, y1, x2, y2, c)
  foreign static f_ellipsefill(x1, y1, x2, y2, c)

 /**
     @Method pset
       Sets the given (x, y) co-ordinate with the Color given.
       @Param
         @Name x
         @Type number
         The x-coordinate of the pixel to set.
       @Param
         @Name y
         @Type number
         The y-coordinate of the pixel to set.
       @Param
         @Name y
         @Type Color | number
         The 32-bit value or Color object representing the color the pixel should be set to.
 */
  static pset(x, y, c) {
    if (c is Color) {
      f_pset(x, y, c.rgb)
    } else {
      f_pset(x, y, c)
    }
  }

  static line(x0, y0, x1, y1, c) {
    if (c is Color) {
      f_line(x0, y0, x1, y1, c.rgb)
    } else {
      f_line(x0, y0, x1, y1, c)
    }
  }
  static ellipse(x0, y0, x1, y1, c) {
    if (c is Color) {
      f_ellipse(x0, y0, x1, y1, c.rgb)
    } else {
      f_ellipse(x0, y0, x1, y1, c)
    }
  }
  static ellipsefill(x0, y0, x1, y1, c) {
    if (c is Color) {
      f_ellipsefill(x0, y0, x1, y1, c.rgb)
    } else {
      f_ellipsefill(x0, y0, x1, y1, c)
    }
  }
  static rect(x, y, w, h, c) {
    if (c is Color) {
      f_rect(x, y, w, h, c.rgb)
    } else {
      f_rect(x, y, w, h, c)
    }
  }
  static rectfill(x, y, w, h, c) {
    if (c is Color) {
      f_rectfill(x, y, w, h, c.rgb)
    } else {
      f_rectfill(x, y, w, h, c)
    }
  }
  static circle(x, y, r, c) {
    if (c is Color) {
      f_circle(x, y, r, c.rgb)
    } else {
      f_circle(x, y, r, c)
    }
  }
  static circlefill(x, y, r, c) {
    if (c is Color) {
      f_circlefill(x, y, r, c.rgb)
    } else {
      f_circlefill(x, y, r, c)
    }
  }
  static print(str, x, y, c) {
    if (!(str is String)) {
      str = str.toString
    }
    var color = Color.white
    if (c is Color) {
      color = c
    }
    f_print(str, x, y, color.rgb)
  }
  static cls() {
    cls(Color.black)
  }
  static cls(c) {
    var color = Color.black
    if (c is Color) {
      color = c
    }
    rectfill(0, 0, Canvas.width, Canvas.height, color.rgb)
  }
  foreign static width
  foreign static height

  static draw(object, x, y) {
    if (object is Drawable) {
      object.draw(x, y)
    }
  }
}

/**
    @Class Color
      An instance of this class represents an RGBA color, which can be passed to Canvas methods.
*/
class Color {
  construct new(r, g, b) {
    _r = r
    _g = g
    _b = b
    _a = 255
  }
  construct new(r, g, b, a) {
    _r = r
    _g = g
    _b = b
    _a = a
  }

  rgb {
    return a << 24 | b << 16 | g << 8 | r
  }

  a { _a }
  r { _r }
  g { _g }
  b { _b }

  static black { AllColors["black"] }
  static darkblue { AllColors["darkblue"] }
  static darkpurple { AllColors["darkpurple"] }
  static darkgreen { AllColors["darkgreen"] }
  static brown { AllColors["brown"] }
  static darkgray { AllColors["darkgray"] }
  static lightgray { AllColors["lightgray"] }
  static white { AllColors["white"] }
  static red { AllColors["red"] }
  static orange { AllColors["orange"] }
  static yellow { AllColors["yellow"] }
  static green { AllColors["green"] }
  static blue { AllColors["blue"] }
  static indigo { AllColors["indigo"] }
  static pink { AllColors["pink"] }
  static peach { AllColors["peach"] }
}

var AllColors = {
  "black": Color.new(0, 0, 0),
  "darkblue": Color.new(29, 43, 83),
  "darkpurple": Color.new(126, 37, 83),
  "darkgreen": Color.new(0, 135, 81),
  "brown": Color.new(171, 82, 54),
  "darkgray": Color.new(95, 87, 79),
  "lightgray": Color.new(194, 195, 199),
  "white": Color.new(255, 255, 255),
  "red": Color.new(255, 0, 77),
  "orange": Color.new(255, 163, 0),
  "yellow": Color.new(255, 236, 39),
  "green": Color.new(0, 228, 54),
  "blue": Color.new(41, 173, 255),
  "indigo": Color.new(131, 118, 156),
  "pink": Color.new(255, 119, 168),
  "peach": Color.new(255, 204, 170)
}

