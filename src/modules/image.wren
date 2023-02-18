class Drawable {
  draw(x, y) {}
}

foreign class DrawCommand is Drawable {
  construct new(image, params, empty) {}

  static parse(image, map) {
    map["foreground"] = (map["foreground"] || Color.white).toNum
    map["background"] = (map["background"] || Color.black).toNum
    map["tint"] = (map["tint"] || Color.none).toNum
    return DrawCommand.new(image, map, null)
  }

  foreign draw(x, y)
  foreign f_modify(modify, empty)
  modify(map) {
    if (map.containsKey("foreground")) {
      map["foreground"] = (map["foreground"] || Color.white).toNum
    }
    if (map.containsKey("background")) {
      map["background"] = (map["background"] || Color.black).toNum
    }
    if (map.containsKey("tint")) {
      map["tint"] = (map["tint"] || Color.none).toNum
    }
    return f_modify(map, null)
  }
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

foreign class AsepriteImage {

  static loadFromFile(path) {
    var data = FileSystem.load(path)
    return AsepriteImage.new(data)

  }
  construct new(fileBuffer) {}
  foreign do()

}

class SpriteSheet {
  static loadFromImage(image, tileSize) {
    return SpriteSheet.new(image, tileSize, 1)
  }
  static loadFromImage(image, tileSize, scale)  {
    return SpriteSheet.new(image, tileSize, scale)
  }

  static loadFromFile(path, tileSize, scale) {
    var image = ImageData.loadFromFile(path)
    return SpriteSheet.new(image, tileSize, scale)
  }
  static loadFromFile(path, tileSize) {
    return SpriteSheet.loadFromFile(path, tileSize, 1)
  }

  construct new(image, tileSize) {
    setup(image, tileSize, 1)
  }

  construct new(image, tileSize, scale) {
    setup(image, tileSize, scale)
  }

  setup(image, tileSize, scale) {
    _image = image
    _tSize = tileSize
    if (_image.width % _tSize != 0) {
      Fiber.abort("Image is not an integer number of tiles wide")
    }
    _width = _image.width / _tSize
    _scale = scale
    _fg = null
    _bg = None
    _cache = {}
  }

  getTile(s) {
    if (!_cache[s]) {
      var sy = (s / _width).floor * _tSize
      var sx = (s % _width).floor * _tSize

      _cache[s] = _image.transform({
        "srcX": sx, "srcY": sy,
        "srcW": _tSize, "srcH": _tSize,
        "mode": _fg ? "MONO" : "RGBA",
        "scaleX": _scale,
        "scaleY": _scale,
        "foreground": _fg || White,
        "background": _bg || None
      })
    }

    return _cache[s]
  }

  draw(s, x, y) { draw(s, x, y, null) }
  draw(s, x, y, modMap) {
    getTile(s).modify(modMap || {
      "foreground": _fg || White,
      "background": _bg || None
    }).draw(x, y)
  }

  fg=(v) { _fg = v }
  fg { _fg }
  bg=(v) { _bg = v }
  bg { _bg }
}

// Aliases
var Bitmap = ImageData
var Image = ImageData

import "color" for Color, None, White
import "io" for FileSystem
