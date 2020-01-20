class Drawable {
  draw(x, y) {}
}

foreign class DrawCommand is Drawable {
  construct new(image, params) {}

  static parse(image, map) {
    var list = [
      map["angle"] || 0,
      map["scaleX"] || 1,
      map["scaleY"] || 1,
      map["srcX"] || 0,
      map["srcY"] || 0,
      map["srcW"] || image.width,
      map["srcH"] || image.height,
    ]
    return DrawCommand.new(image, list)
  }

  foreign draw(x, y)

}


foreign class ImageData is Drawable {
  // This constructor is private
  construct initFromFile(data) {}

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
  foreign draw(x, y)
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

  foreign width
  foreign height
}

