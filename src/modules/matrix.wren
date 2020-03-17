
class Matrix {
  construct new(width, height) {
    _width = width
    _height = height
    _data = List.filled(0, width * height)
  }

  construct new(dim) {
    _width = dim
    _height = dim
    _data = List.filled(0, dim.pow(2))
  }

  width { _width }
  height { _height }
  [index] { _data[index] }
  toList { _data }

  toString {
    return "[%(),%(),%()\n %(),%(),%()\n %(),%(),%()]"
  }

}
