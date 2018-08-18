class Canvas {
 foreign static f_pset(x, y, c)
 foreign static f_line(x1, y1, x2, y2, c)
 foreign static f_rectfill(x, y, w, h, c)
 foreign static f_rect(x, y, w, h, c)
 foreign static f_print(str, x, y, c)
 foreign static f_circle(x, y, r, c)
 foreign static f_circlefill(x, y, r, c)
 foreign static f_ellipse(x1, y1, x2, y2, c)
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
 static width { 320 }
 static height { 240 }

 static draw(object, x, y) {
   if (object is ImageData) {
     object.draw(x, y)
   }
 }
}

class Color {
  construct new(r, g, b) {
    _r = r
    _g = g
    _b = b
    _a = 255
  }

  rgb { Color.rgb(_r, _g, _b, _a) }

  static white { AllColors["white"] }
  static black { AllColors["black"] }
  static red { AllColors["red"] }
  static orange { AllColors["orange"] }
  static blue { AllColors["blue"] }
  static green { AllColors["green"] }
  static cyan { AllColors["cyan"] }
  static darkgray { AllColors["darkgray"] }
  static lightgray { AllColors["lightgray"] }

  static rgb(r, g, b, a) {
    return a << 24 | r << 16 | g << 8 | b
  }
}

var AllColors = {
  "black": Color.new(0, 0, 0),
  "white": Color.new(255, 255, 255),
  "orange": Color.new(255, 163, 0),
  "red": Color.new(255, 0, 0),
  "green": Color.new(0, 255, 0),
  "blue": Color.new(0, 0, 255),
  "cyan": Color.new(0, 255, 255),
  "magenta": Color.new(255, 0, 255),
  "yellow": Color.new(255, 255, 0),
  "lightgray": Color.new(194, 195, 199),
  "darkgray": Color.new(95, 87, 79)
}

foreign class ImageData {
  construct fromFile(path) {}
  static loadFromFile(path) {
    if (!__cache) {
      __cache = {}
    }

    if (!__cache.containsKey(path)) {
      __cache[path] = ImageData.fromFile(path)
    }

    return __cache[path]
  }
  foreign draw(x, y)
}

foreign class Point {
  construct new() {}
  construct new(x, y) {}
  foreign x
  foreign x=(v)
  foreign y
  foreign y=(v)
}
