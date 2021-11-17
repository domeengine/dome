foreign class Raycaster {
  construct init() {}
  foreign setPosition(x, y)
  foreign setAngle(angle)
  foreign setDimensions(width, height)

  foreign draw(alpha)
  foreign update()
  foreign loadTexture(path)

  foreign f_pushObject(x, y, textureId)
  tile(x, y) {
    return WorldTile.init(this, x, y)
  }

  object(id) {
    // Should we create an object if the id is not used?
    return WorldObject.init(this, id)
  }

  pushObject(x, y, textureId) {
    var id = f_pushObject(x, y, textureId)
    return WorldObject.init(this, id)
  }
}

foreign class WorldObject {
  construct init(renderer, id) {}

  foreign remove()

  // TODO: Fluent API
  foreign id

  foreign textureId
  foreign textureId=(v)
  foreign x
  foreign x=(v)
  foreign y
  foreign y=(v)
  foreign uDiv
  foreign uDiv=(v)
  foreign vDiv
  foreign vDiv=(v)
  foreign vMove
  foreign vMove=(v)
}

foreign class WorldTile {
  construct init(renderer, x, y) {}

  thin(value) {
    thin = value
    return this
  }

  offset(value) {
    offset = value
    return this
  }

  wallTextureId(value) {
    wallTextureId = value
    return this
  }

  ceilingTextureId(value) {
    ceilingTextureId = value
    return this
  }

  floorTextureId(value) {
    floorTextureId = value
    return this
  }

  solid(value) {
    solid = value
    return this
  }

  state(value) {
    state = value
    return this
  }

  mode(value) {
    mode = value
    return this
  }

  door(value) {
    door = value
    return this
  }

  foreign wallTextureId
  foreign wallTextureId=(v)
  foreign ceilingTextureId
  foreign ceilingTextureId=(v)
  foreign floorTextureId
  foreign floorTextureId=(v)
  foreign solid
  foreign solid=(v)
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
