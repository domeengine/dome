foreign class Raycaster {
  construct init() {}
  foreign setPosition(x, y)
  foreign setAngle(angle)
  foreign draw(alpha)
  foreign update()
  foreign loadTexture(path)

  foreign f_pushObject(x, y, textureId)
  pushObject(x, y, textureId) {
    var id = f_pushObject(x, y, textureId)
//     return WorldObject.init(this, id)
  }
}

foreign class WorldObject {
  construct init(renderer, id) {}
  // foreign textureId
  // foreign textureId=(v)
  // foreign setPosition(x, y)
  // TODO: set div, vMove
}

foreign class WorldTile {
  construct init(renderer, x, y) {}
  foreign solid
  foreign solid=(v)
  foreign setTextures(wall)
  foreign setTextures(wall, floor)
  foreign setTextures(wall, floor, ceiling)
  foreign door
  foreign door=(v)
  foreign state
  foreign state=(v)
  foreign mode
  foreign mode=(v)
  foreign offset
  foreign offset=(v)
  foreign thin
  foreign thin=(v)
}
