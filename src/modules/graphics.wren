class Canvas {
  static init_() {
    __defaultFont = null
  }

  static clip() { clip(0, 0, Canvas.width, Canvas.height) }
  foreign static clip(x, y, width, height)

  static font=(v) {
    if (v is String || v == Font.default) {
      __defaultFont = v
    } else {
      Fiber.abort("Default font must be a font name")
    }
  }

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
    clip()
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
  foreign static f_line(x1, y1, x2, y2, c, size)
  foreign static f_rectfill(x, y, w, h, c)
  foreign static f_rect(x, y, w, h, c)
  foreign static f_print(str, x, y, c)
  foreign static f_circle(x, y, r, c)
  foreign static f_circlefill(x, y, r, c)
  foreign static f_ellipse(x1, y1, x2, y2, c)
  foreign static f_ellipsefill(x1, y1, x2, y2, c)

  static pset(x, y, c) {
    if (c is Color) {
      f_pset(x, y, c.toNum)
    } else {
      f_pset(x, y, c)
    }
  }

  static line(x0, y0, x1, y1, c) { line(x0, y0, x1, y1, c, 1) }
  static line(x0, y0, x1, y1, c, size) {
    if (c is Color) {
      f_line(x0, y0, x1, y1, c.toNum, size)
    } else {
      f_line(x0, y0, x1, y1, c, size)
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
  static print(str, x, y, c, font) {
    if (Font[font] != null) {
      Font[font].print(str, x, y, c)
    } else {
      Fiber.abort("Font %(font) is not loaded")
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
    if (__defaultFont != null) {
      print(str, x, y, color, __defaultFont)
    } else {
      f_print(str, x, y, color.toNum)
    }
  }

  foreign static f_cls(color)
  static cls() {
    cls(Color.black)
  }
  static cls(c) {
    var color = Color.black
    if (c is Color) {
      color = c
    }
    f_cls(color.toNum)
  }
  foreign static width
  foreign static height

  static draw(object, x, y) {
    if (object is Drawable) {
      object.draw(x, y)
    }
  }
}


// These need to be at the bottom to prevent cyclic dependancy
import "color" for Color
import "image" for Drawable, ImageData
import "vector" for Point, Vec, Vector
import "font" for Font, RasterizedFont

Canvas.init_()
