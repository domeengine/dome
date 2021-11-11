foreign class Raycaster {
  construct init() {}
  foreign setPosition(x, y)
  foreign setAngle(angle)
  foreign draw(alpha)
  foreign loadTexture(path)
}

foreign class WorldTile {
  construct init(renderer, x, y) {}
  foreign solid
  foreign solid=(v)
  foreign setTextures(wall)
  foreign setTextures(wall, floor)
  foreign setTextures(wall, floor, ceiling)
}
