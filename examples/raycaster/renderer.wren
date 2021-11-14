foreign class Raycaster {
  construct init() {}
  foreign setPosition(x, y)
  foreign setAngle(angle)
  foreign draw(alpha)
  foreign update()
  foreign loadTexture(path)
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
