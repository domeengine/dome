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
  foreign f_draw(text, color, size)
  draw(text, color, size) {
    f_draw(text, color.toNum, size)
  }
}

foreign class Font {
  static initFontCache_() {
    __fontFiles = {}
    __rasterizedFonts = {}
  }

  static load(name, path) {
    __fontFiles[path] = FontFile.parse(FileSystem.load(path))
    return __fontFiles[path]
    // var font = RasterizedFont.parse(fontFile, size)
  }

  static [index] { __rasterizedFonts[index] }
}

Font.initFontCache_()
