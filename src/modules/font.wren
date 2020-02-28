import "io" for FileSystem

/*
-----

Font.load("map", "font.ttf", 6)
Font.unload("map")

Canvas.font = "map"
Canvas.font = Font.default

Canvas.print("Worlds in the main set", x, y, color)
Canvas.print("Words in the font", x, y, color, font)

-----

*/

foreign class FontFile {
  construct parse(data) {}
}


foreign class RasterizedFont {
  construct rasterize(file, size) {}
}

foreign class Font {
  static initFontCache_() {
    __fontFiles = {}
    __rasterizedFonts = {}
  }

  static load(name, path, pixelSize) {
    __fontFiles[path] = FontFile.parse(FileSystem.load(path))
    // var font = RasterizedFont.parse(fontFile, size)
  }

  static [index] { __rasterizedFonts[index] }
}

Font.initFontCache_()
