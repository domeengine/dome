foreign class ImageData {
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
  foreign drawArea(srcX, srcY, srcW, srcH, destX, destY)

  foreign width
  foreign height
}

