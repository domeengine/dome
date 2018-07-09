import "input" for Keyboard
import "graphics" for Canvas, Color, ImageData
// The default random module seems to be broken currently
// import "random" for Random

class Random {
  construct new(seed) {
    _seed = (seed % 2147483646).abs % 2147483647
    if (_seed <= 0) {
      _seed = _seed + 2147483646
    }
  }

  int(n) {
   _seed = _seed * 16807 % 2147483647
   return _seed % n
  }
}

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
  construct new(x, y) {
    _x = x
    _y = y
    _image = ImageData.loadFromFile("img/enemy.png")
  }

  x { _x }
  y { _y }

  update() {
    _y = _y + 1
  }

  draw() {
    Canvas.draw(_image, x, y)
  }
}


class Ship {
  construct new() {
    _x = Canvas.width / 2
    _y = Canvas.height - 20
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
    __enemies = []
    var random = Random.new(12345)
    for (i in 0...5) {
      __enemies.add(Enemy.new(random.int(Canvas.width), -random.int(30)))
    }
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

    i = 0
    for (enemy in __enemies) {
      if (enemy.y > Canvas.height) {
        __enemies.removeAt(i)
      }
      enemy.update()
      i = i + 1
    }
  }

  static draw(dt) {
    Canvas.cls()
    Canvas.print("Hello world", 0, 0, Color.white)
    /*
    var color = Color.new(171, 82, 54).rgb
    Canvas.rectfill(__x, __y, __w, __h, color)
    */

    __ship.draw(__t)
    for (bullet in __bullets) {
      bullet.draw()
    }
    for (enemy in __enemies) {
      enemy.draw()
    }

  }
}
