import "input" for Keyboard
import "graphics" for Canvas, Color, ImageData

class Bullet {

  construct fire(x, y) {
    _x = x
    _y = y
  }
  y { _y }

  update() {
    _y = _y - 1
  }

  draw() {
    var color = Color.white
    Canvas.rectfill(_x, _y, 2, 2, color)
  }
}

class Enemy {
  construct new() {
    _x = 10
    _y = 10
    _image = ImageData.loadFromFile("img/enemy.png")
  }

  x { _x }
  y { _y }

  move() {
    _y = _y + 1
  }

  draw() {
    Canvas.draw(_image, x, y)
  }
}


class Ship {
  construct new() {
    _x = 10
    _y = 10
    _t = 0
    _ship = [
      ImageData.loadFromFile("img/ship1.png"),
      ImageData.loadFromFile("img/ship2.png")
    ]
  }

  x { _x }
  y { _y }

  move(x, y) {
    _x = _x + x
    _y = _y + y
  }

  draw(t) {
    var frame = (t / 5).floor % 2
    Canvas.draw(_ship[frame], _x, _y)
  }
}

class Game {

  static init() {
    __w = 5
    __h = 5
    __t = 0

    __ship = Ship.new()
    __bullets = []
    __lastFire = 0


  }

  static update() {
    __t = __t + 1
    var x = 0
    var y = 0
    if (Keyboard.isKeyDown("left")) {
      x = -1
    }
    if (Keyboard.isKeyDown("right")) {
      x = 1
    }
    if (Keyboard.isKeyDown("up")) {
      y = -1
    }
    if (Keyboard.isKeyDown("down")) {
      y = 1
    }
    if (Keyboard.isKeyDown("space") && (__t - __lastFire) > 10) {
      __bullets.add(Bullet.fire(__ship.x+2, __ship.y))
      __lastFire = __t
    }

    __ship.move(x, y)
    var i = 0
    for (bullet in __bullets) {
      if (bullet.y < 0) {
        __bullets.removeAt(i)
      }
      bullet.update()
      i = i + 1
    }
  }

  static draw(dt) {
    Canvas.cls()
    Enemy.new().draw()
    /*
    var color = Color.new(171, 82, 54).rgb
    Canvas.rectfill(__x, __y, __w, __h, color)
    */

    __ship.draw(__t)
    for (bullet in __bullets) {
      bullet.draw()
    }

  }
}
