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
    _alive = true
  }
  x { _x }
  y { _y }
  h { 2 }
  w { 2 }
  alive { _alive }

  kill() {
    _alive = false
  }

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
    _alive = true
    _image = ImageData.loadFromFile("img/enemy.png")
  }

  alive { _alive }
  x { _x }
  y { _y }
  h { 8 }
  w { 6 }

  kill() {
    _alive = false
  }

  update() {
    _y = _y + 1
  }

  draw() {
    if (alive) {
      Canvas.draw(_image, x, y)
    }
  }
}


class Ship {
  construct new() {
    _x = Canvas.width / 2
    _y = Canvas.height - 20
    _health = 3
    _t = 0
    _ship = [
      ImageData.loadFromFile("img/ship1.png"),
      ImageData.loadFromFile("img/ship2.png")
    ]
  }

  x { _x }
  y { _y }
  h { 8 }
  w { 6 }
  health { _health }

  damage() {
    _health = _health - 1
  }

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
    __points = 0

    __ship = Ship.new()
    __bullets = []
    __enemies = []

    __random = Random.new(12345)
    for (i in 0...5) {
      __enemies.add(Enemy.new(__random.int(Canvas.width), -__random.int(30)))
    }
    __lastFire = 0
    __heart = ImageData.loadFromFile("img/heart-full.png")
    __heartEmpty = ImageData.loadFromFile("img/heart-empty.png")
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

    for (enemy in __enemies) {
      enemy.update()
      if (colliding(__ship, enemy)) {
        __ship.damage()
        enemy.kill()
      }
    }

    var bulletCount = 0
    for (bullet in __bullets) {
      bullet.update()

      // check if we hit something
      for (enemy in __enemies) {
        if (enemy.alive && colliding(bullet, enemy)) {
          bullet.kill()
          enemy.kill()
          __points = __points + 1
        }
      }
    }

    __bullets = __bullets.where {|bullet|
      return bullet.alive && bullet.y > 0
    }.toList
    __enemies = __enemies.where {|enemy|
      var isAlive = enemy.alive && enemy.y < Canvas.height
      if (!isAlive) {
        __enemies.add(Enemy.new(__random.int(Canvas.width), 0))
      }
      return isAlive
    }.toList
  }

  static colliding(o1, o2) {
    var box1 = Box.new(o1.x, o1.y, o1.x + o1.w, o1.y+o1.h)
    var box2 = Box.new(o2.x, o2.y, o2.x + o2.w, o2.y+o2.h)
    return box1.x1 < box2.x2 &&
      box1.x2 > box2.x1 &&
      box1.y1 < box2.y2 &&
      box1.y2 > box2.y1
  }

  static draw(dt) {
    Canvas.cls()

    __ship.draw(__t)
    for (bullet in __bullets) {
      bullet.draw()
    }
    for (enemy in __enemies) {
      enemy.draw()
    }

    Canvas.rectfill(0, 0, 320, 10, Color.black)

    for (i in 1..3) {
      if (i <= __ship.health) {
        Canvas.draw(__heart, 292+6*i, 3)
      } else {
        Canvas.draw(__heartEmpty, 292+6*i, 3)
      }
    }

    Canvas.print("Score: %(__points)", 3, 3, Color.white)
  }
}

class Box {
  construct new(x1, y1, x2, y2) {
    _x1 = x1
    _x2 = x2
    _y1 = y1
    _y2 = y2
  }

  x1 { _x1 }
  y1 { _y1 }
  x2 { _x2 }
  y2 { _y2 }

}
