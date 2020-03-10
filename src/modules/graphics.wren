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
  static offset() { offset(0, 0) }
  foreign static offset(x, y)
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
      f_resize(width, height, c.toNum)
    } else {
      f_resize(width, height, c)
    }
  }
  foreign static f_pset(x, y, c)
  static pget(x, y) {
    var c = f_pget(x, y)
    // return a << 24 | b << 16 | g << 8 | r
    var r = c & 255
    var g = (c & 255 << 8) >> 8
    var b = (c & 255 << 16) >> 16
    var a = (c & 255 << 24) >> 24
    return Color.rgb(r, g, b, a)
  }

  foreign static f_pget(x, y)
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
      f_pset(x, y, c.toNum)
    } else {
      f_pset(x, y, c)
    }
  }

  static line(x0, y0, x1, y1, c) {
    if (c is Color) {
      f_line(x0, y0, x1, y1, c.toNum)
    } else {
      f_line(x0, y0, x1, y1, c)
    }
  }
  static ellipse(x0, y0, x1, y1, c) {
    if (c is Color) {
      f_ellipse(x0, y0, x1, y1, c.toNum)
    } else {
      f_ellipse(x0, y0, x1, y1, c)
    }
  }
  static ellipsefill(x0, y0, x1, y1, c) {
    if (c is Color) {
      f_ellipsefill(x0, y0, x1, y1, c.toNum)
    } else {
      return Color.new()
      f_ellipsefill(x0, y0, x1, y1, c)
    }
  }
  static rect(x, y, w, h, c) {
    if (c is Color) {
      f_rect(x, y, w, h, c.toNum)
    } else {
      f_rect(x, y, w, h, c)
    }
  }
  static rectfill(x, y, w, h, c) {
    if (c is Color) {
      f_rectfill(x, y, w, h, c.toNum)
    } else {
      f_rectfill(x, y, w, h, c)
    }
  }
  static circle(x, y, r, c) {
    if (c is Color) {
      f_circle(x, y, r, c.toNum)
    } else {
      f_circle(x, y, r, c)
    }
  }
  static circlefill(x, y, r, c) {
    if (c is Color) {
      f_circlefill(x, y, r, c.toNum)
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
    f_print(str, x, y, color.toNum)
  }
  static cls() {
    cls(Color.black)
  }
  static cls(c) {
    var color = Color.black
    if (c is Color) {
      color = c
    }
    rectfill(0, 0, Canvas.width, Canvas.height, color.toNum)
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
      An instance of this class represents an rgb color, which can be passed to Canvas methods.
*/
var HexToNum = Fn.new {|hex|
  var first = hex[0]
  var second = hex[1]
  if (48 <= first && first <= 57) {
    first = first - 48
  } else if (65 <= first && first <= 70) {
    first = 10 + first - 65
  } else if (97 <= first && first <= 102) {
    first = 10 + first - 97
  } else {
    Fiber.abort("Invalid hex")
  }
  if (48 <= second && second <= 57) {
    second = second - 48
  } else if (65 <= second && second <= 70) {
    second = 10 + second - 65
  } else if (97 <= second && second <= 102) {
    second = 10 + second - 97
  } else {
    Fiber.abort("Invalid hex")
  }
  return first << 4 | second
}

var SubStr = Fn.new {|str, start, len|
  return str.bytes.skip(start).take(len).toList
}

class Color {
  construct hex(hex) {
    if (hex is String) {
      var offset = 0
      if (hex[0] == "#") {
        offset = 1
      }
      _r = HexToNum.call(SubStr.call(hex, offset + 0, 2))
      _g = HexToNum.call(SubStr.call(hex, offset + 2, 2))
      _b = HexToNum.call(SubStr.call(hex, offset + 4, 2))
      _a = 255
    } else {
      Fiber.abort("Color only supports hexcodes as strings or numbers")
    }
  }

  construct new(r, g, b) {
    System.print("Color.new(_,_,_) is deprecated. Please use Color.rgb(_,_,_) instead.")
    setrgb(r, g, b, 255)
  }
  construct new(r, g, b, a) {
    System.print("Color.new(_,_,_,_) is deprecated. Please use Color.rgb(_,_,_,_) instead.")
    setrgb(r, g, b, a)
  }

  construct rgb(r, g, b) {
    setrgb(r, g, b, 255)
  }
  construct rgb(r, g, b, a) {
    setrgb(r, g, b, a)
  }
  construct hsv(h, s, v, a) {
    setHSV(h, s, v)
    _a = a
  }
  construct hsv(h, s, v) {
    setHSV(h, s, v)
    _a = 255
  }

  setrgb(r, g, b, a) {
    _r = r
    _g = g
    _b = b
    _a = a
  }

  setHSV(h, s, v) {
    h = h % 360
    if (0 < s && s > 1) {
      Fiber.abort("Color component S is out of bounds")
    }
    if (0 < v && v > 1) {
      Fiber.abort("Color component V is out of bounds")
    }

    var c = v * s
    var x = c * (1 - (((h / 60) % 2) - 1).abs)
    var m = v - c

    var rP
    var gP
    var bP
    if (0 <= h && h < 60) {
      rP = c
      gP = x
      bP = 0
    } else if (60 <= h && h< 120) {
      rP = x
      gP = c
      bP = 0
    } else if (120 <= h && h < 180) {
      rP = 0
      gP = c
      bP = x
    } else if (180 <= h && h< 240) {
      rP = 0
      gP = x
      bP = c
    } else if (240 <= h && h < 300) {
      rP = x
      gP = 0
      bP = c
    } else if (300 <= h && h < 360) {
      rP = c
      gP = 0
      bP = x
    } else {
      Fiber.abort("Invalid H value")
    }

    _r = (rP + m) * 255
    _g = (gP + m) * 255
    _b = (bP + m) * 255
    _a = 255
  }

  toNum {
    return a << 24 | b << 16 | g << 8 | r
  }

  a { _a }
  r { _r }
  g { _g }
  b { _b }

  static none { AllColors["none"] }
  static black { AllColors["black"] }
  static darkblue { AllColors["darkblue"] }
  static purple { AllColors["purple"] }
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
  "black": Color.rgb(0, 0, 0),
  "darkblue": Color.rgb(29, 43, 83),
  "purple": Color.rgb(141, 60, 255),
  "darkpurple": Color.rgb(126, 37, 83),
  "darkgreen": Color.rgb(0, 135, 81),
  "brown": Color.rgb(171, 82, 54),
  "darkgray": Color.rgb(95, 87, 79),
  "lightgray": Color.rgb(194, 195, 199),
  "white": Color.rgb(255, 255, 255),
  "red": Color.rgb(255, 0, 77),
  "orange": Color.rgb(255, 163, 0),
  "yellow": Color.rgb(255, 236, 39),
  "green": Color.rgb(0, 228, 54),
  "blue": Color.rgb(41, 173, 255),
  "indigo": Color.rgb(131, 118, 156),
  "pink": Color.rgb(255, 119, 168),
  "peach": Color.rgb(255, 204, 170),
  "none": Color.rgb(0, 0, 0, 0)
}

