class Graphics {
 foreign static pset(x, y, c)
 static screenHeight { 240 }
 static screenWidth { 320 }
}

class Color {
  static rgb(r, g, b, a) {
    return a << 24 | r << 16 | g << 8 | b
  }
}

System.print("Graphics initialized.")
