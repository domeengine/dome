import "io" for FileSystem

foreign class FontFile {
  construct parse(data) {}
}

class Font {
  static initFontCache_() {
    __fontFiles = {}
    __rasterizedFonts = {}
  }

  static default { null }

  static load(name, path, size) {
    if (!__fontFiles.containsKey(path)) {
      __fontFiles[path] = FontFile.parse(FileSystem.load(path))
    }
    __rasterizedFonts[name] = RasterizedFont.parse(__fontFiles[path], size)
    return __rasterizedFonts[name]
  }

  static unload(name) {
    __rasterizedFonts.remove[name]
    // TODO: Check if we are using the font and unload that too?
  }

  static [index] { __rasterizedFonts[index] }
}

Font.initFontCache_()

foreign class RasterizedFont is Font {
  construct parse(file, size) {}
  foreign antialias=(v)
  foreign f_print(text, x, y, color)

  print(text, x, y, color) {
    f_print(text, x, y, color.toNum)
  }
}

