class Drawable {
  draw(x, y) {}
}

foreign class DrawCommand is Drawable {
  construct new(image, params, empty) {}

  static parse(image, map) {
    map["foreground"] = (map["foreground"] || Color.white).toNum
    map["background"] = (map["background"] || Color.black).toNum
    map["tint"] = (map["tint"] || Color.none).toNum
    System.print(map)
    return DrawCommand.new(image, map, null)
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
  pget(x, y) { Color.fromNum(f_pget(x, y)) }
}

import "color" for Color
import "io" for FileSystem
