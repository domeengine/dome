import "color" for Color
class Drawable {
  draw(x, y) {}
}

foreign class DrawCommand is Drawable {
  construct new(image, params) {}

  static parse(image, map) {
    import "graphics" for Color
    var list = [
      map["angle"] || 0,
      map["scaleX"] || 1,
      map["scaleY"] || 1,
      map["srcX"] || 0,
      map["srcY"] || 0,
      map["srcW"] || image.width,
      map["srcH"] || image.height,
      map["mode"] || "RGBA",
      (map["foreground"] || Color.white).toNum,
      (map["background"] || Color.black).toNum
    ]
    return DrawCommand.new(image, list)
  }

  foreign draw(x, y)
}


foreign class ImageData is Drawable {
  // Creates an empty image
  construct init(width, height) {}

  // This constructor is private
  construct initFromFile(data) {
    f_loadFromFile(data)
  }
  foreign f_loadFromFile(data)

  foreign f_getPNG()
  saveToFile(path) {
    import "io" for FileSystem
    var data = f_getPNG()
    if (data != null) {
      FileSystem.save(path, data)
    }
  }

  static create(name, width, height) {
    if (!__cache) {
      __cache = {}
    }
    if (!__cache.containsKey(name)) {
      __cache[name] = ImageData.init(width, height)
    }
    return __cache[name]
  }

  static [name] {
    if (!__cache) {
      __cache = {}
    }
    return __cache[name]
  }

  static loadFromFile(path) {
    if (!__cache) {
      __cache = {}
    }

    if (!__cache.containsKey(path)) {
      import "io" for FileSystem
      var data = FileSystem.load(path)
      __cache[path] = ImageData.initFromFile(data)
    }

    return __cache[path]
  }
  transform(map) {
    return DrawCommand.parse(this, map)
  }

  drawArea(srcX, srcY, srcW, srcH, destX, destY) {
    return this.transform({
      "srcX": srcX,
      "srcY": srcY,
      "srcW": srcW,
      "srcH": srcH
    }).draw(destX, destY)
  }

  foreign draw(x, y)
  foreign width
  foreign height

  foreign f_pset(x, y, color)
  pset(x, y, c) {
    if (c is Color) {
      f_pset(x, y, c.toNum)
    } else {
      f_pset(x, y, c)
    }
  }

  foreign f_pget(x, y)
  pget(x, y) {
    var c = f_pget(x, y)
    var r = c & 255
    var g = (c & 255 << 8) >> 8
    var b = (c & 255 << 16) >> 16
    var a = (c & 255 << 24) >> 24
    return Color.rgb(r, g, b, a)
  }
}

