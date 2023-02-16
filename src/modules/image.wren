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

class ImageSheet {

  construct new(path, tileSize) {
    setup(path, tileSize, 1)
  }

  construct new(path, tileSize, scale) {
    setup(path, tileSize, scale)
  }

  setup(path, tileSize, scale) {
    _image = ImageData.loadFromFile(path)
    _tSize = tileSize
    if (_image.width % _tSize != 0) {
      Fiber.abort("Image is not an integer number of tiles wide")
    }
    _width = _image.width / _tSize
    _scale = scale
    _fg = null
    _bg = null
    _cache = {}
  }

  draw(s, x, y) { getTile(s).draw(x, y) }

  getTile(s) {
    if (!_cache[s]) {
      var sy = (s / _width).floor * _tSize
      var sx = (s % _width).floor * _tSize

      var transform = _image.transform({
        "srcX": sx, "srcY": sy,
        "srcW": _tSize, "srcH": _tSize,
        "mode": _fg ? "MONO" : "RGBA",
        "scaleX": _scale,
        "scaleY": _scale,
        "foreground": _fg || White,
        "background": _bg || None
      })
      _cache[s] = transform
    }

    return _cache[s]
  }
}

import "color" for Color, None, White
import "io" for FileSystem
