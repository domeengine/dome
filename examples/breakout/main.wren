// title: breakout clone
// author: camilo castro (clsource)
// desc: a breakout clone in wren
// script: wren
// based on: https://github.com/digitsensitive/tic-80-tutorials/tree/master/tutorials/breakout
// original: https://github.com/NinjasCL/breakout
// license: mit

import "random" for Random
import "graphics" for Canvas, Color
import "input" for Keyboard
import "dome" for Window

var Rand = Random.new()

// TIC-80 Width and Height
class ScreenWidth {
    static min {0}
    static max {240}
}

class ScreenHeight {
    static min {0}
    static max {136}
}

class Screen {
    static height {ScreenHeight}
    static width {ScreenWidth}
}

class Settings {
  static scale {4}
  static title {"::Breakout::"}

  static init() {
    Canvas.resize(Screen.width.max, Screen.height.max)
    Window.resize(scale * Canvas.width, scale * Canvas.height)
    Window.title = title
  }
}

class Input {
    static left {Keyboard.isKeyDown("left")}
    static right {Keyboard.isKeyDown("right")}
    static x {Keyboard.isKeyDown("x")}
}

class Colors {
    static black {Color.hex("1A1C2C")}
    static white {Color.white}
    static orange {Color.orange}
    static greyl {Color.lightgray}
    static greyd {Color.darkgray}

    // https://github.com/nesbox/TIC-80/wiki/palette
    static palette {[
      Color.hex("1A1C2C"), // black
      Color.hex("5D275D"), // purple
      Color.hex("B13E53"), // red
      Color.hex("EF7D57"), // orange
      Color.hex("FFCD75"), // yellow
      Color.hex("A7F070"), // light green
      Color.hex("38B764"), // green
      Color.hex("257179"), // dark green
      Color.hex("29366F"), // dark blue
      Color.hex("3B5DC9"), // blue
      Color.hex("41A6F6"), // light blue
      Color.hex("73EFF7"), // cyan
      Color.hex("F4F4F4"), // white
      Color.hex("94B0C2"), // light grey
      Color.hex("566C86"), // grey
      Color.hex("333C57")  // dark grey
    ]}
}

class Collisions {
  // Implements
  // https://developer.mozilla.org/en-US/docs/Games/Techniques/2D_collision_detection
  static collide(hitbox, hitbox2) {
    return (hitbox.x < hitbox2.x + hitbox2.width &&
            hitbox.x + hitbox.width > hitbox2.x &&
            hitbox.y < hitbox2.y + hitbox2.height &&
            hitbox.y + hitbox.height > hitbox2.y)
  }
}

class PlayerSpeed {
    x {_x}
    x = (value) {
        _x = value
    }
    
    max {_max}
    max = (value) {
        _max = value
    }
    
    construct new() {
        _x = 0
        _max = 4
    }
}

class Player {
    x {_x}
    y {_y}
    width {_width}
    height {_height}
    color {_color}
    speed {_speed}

    state {_state}

    construct new(state) {
        _state = state
        _width = 24
        _height = 4
        _y = 120
        _color = Colors.orange
        reset()
    }

    reset() {
        _x = (Screen.width.max/2) - _width/2
        _speed = PlayerSpeed.new()
    }

    draw() {
        Canvas.rectfill(x, y, width, height, color)
    }

    wall() {
        if (x < Screen.width.min) {
            _x = Screen.width.min
        } else if(x + width > Screen.width.max) {
            _x = Screen.width.max - width
        }
    }

    collisions() {
        wall()
    }

    update() {
        _x = x + speed.x
        if (speed.x != 0) {
            if (speed.x > 0) {
              speed.x = speed.x - 1
            } else {
              speed.x = speed.x + 1
            }
        }
    }

    input() {
        if (Input.left) {
            if (speed.x > -speed.max) {
                speed.x = speed.x - 2
            } else {
                speed.x = -speed.max
            }
        }
            
        if (Input.right) {
          if (speed.x < speed.max) {
                  speed.x = speed.x + 2
          } else {
              speed.x = speed.max
          }
        }
    }
}

class BallSpeed {
    x {_x}
    x = (value) {
        _x = value
    }

    y {_y}
    y = (value) {
        _y = value
    }

    max {_max}

    construct new() {
        _x = 0
        _y = 0
        _max = 1.5
    }
}

class Ball {
    x {_x}
    y {_y}
    width {_width}
    height {_height}
    color {_color}
    deactive {_deactive}
    speed {_speed}

    player {_player}
    player = (value) {
        _player = value
    }

    state {_state}

    construct new(player, state) {
        _width = 3
        _height = 3
        _color = Colors.greyl
        _player = player
        _state = state
        reset()
    }
    
    position() {
        _x = player.x + (player.width / 2) - 1.5
        _y = player.y - 5
    }
    
    reset() {
        position()
        _deactive = true
        _speed = BallSpeed.new()
    }
    
    input() {
        if (deactive) {
            position()
            if (Input.x) {
                speed.x = (Rand.float(0, 10).floor * 2) - 1
                speed.y = speed.y - 1.5
                _deactive = false
            }
        }
    }
    
    wall() {
        // top
        if (y < 0) {
            speed.y = -speed.y
            
        // left
        } else if (x < 0) {
            speed.x = -speed.x
        
        // right
        } else if (x > 240 - width) {
            speed.x = -speed.x
        }
    }
    
    ground() {
        if (y > 136 - width) {
                reset()
                state.lifeDown()
        }
    }
    
    paddle() {
        if (Collisions.collide(this, player)) {
            speed.y = -speed.y
            speed.x = speed.x + 0.3 * player.speed.x
        }
    }

    brick(brick) {

        // collide left or right side
        if (brick.y < y &&
            y < brick.y + brick.height &&
            (x < brick.x || brick.x + brick.width < x)) {
                speed.x = -speed.x
        }

        // collide top or bottom
        if (y < brick.y ||
        (y > brick.y && brick.x < x &&
        x < brick.x + brick.width)) {
            speed.y = -speed.y
        }
    }

    collisions() {
        wall()
        ground()
        paddle()
    }
    
    update() {
        _x = x + speed.x
        _y = y + speed.y
        
        if (speed.x > speed.max) {
            speed.x = speed.max
        }
    }
    
    draw() {
        Canvas.rectfill(x, y, width, height, color)
    }
}

class Brick {
    x {_x}
    y {_y}
    width {_width}
    height {_height}
    color {_color}

    construct new(x, y, color) {
        _x = x
        _y = y
        _width = 10
        _height = 4
        _color = Colors.palette[color]
    }

    draw() {
        Canvas.rectfill(x, y, width, height, color)
    }
}

class Board {
    width {19}
    height {12}

    ball {_ball}
    state {_state}

    bricks {
        if (!_bricks) {
            _blicks = []
        }

        return _bricks
    }

    construct new(ball, state) {
        _ball = ball
        _state = state
        reset()
    }

    reset() {
        _bricks = []
        for (i in 0..height) {
            for (j in 0..width) {
                var x = 10 + j * 11
                var y = 10 + i * 5
                var color = i + 1
                var brick = Brick.new(x, y, color)
                bricks.add(brick)
            }
        }
        ball.reset()
    }

    draw() {
        bricks.each {|brick|
            brick.draw()
        }
    }

    collisions() {
        var index = 0
        if (bricks.count <= 0) {
            reset()
        }

        bricks.each {|brick|
            if (Collisions.collide(ball, brick)) {
                bricks.removeAt(index)
                ball.brick(brick)
                state.scoreUp()
            }
            index = index + 1
        }
    }

    update() {}
    input() {}
}

class Stage {
    objects {_objects}
    state {_state}

    construct new(state) {
        _objects = []
        _state = state
    }

    add(object) {
        objects.add(object)
    }

    input() {
        if (!state.isPlaying) return

        objects.each {|object|
            object.input()
        }
    }

    draw() {
        if (!state.isPlaying) return

        objects.each {|object|
            object.draw()
        }
    }

    update() {
    
        if (!state.isPlaying) return
        
        objects.each {|object|
            object.update()
        }
    }
    
    collisions() {
        if (!state.isPlaying) return 
        
        objects.each {|object|
            object.collisions()
        }
    }
}

class GUI {
    player {_player}
    state {_state}
    
    construct new(player, state) {
        _player = player
        _state = state
    }
    
    scores() {
       // shadow
        Canvas.print("SCORE %(state.score)", 5, 1, Colors.greyd)
        
        // forecolor
        Canvas.print("SCORE %(state.score)", 5, 0, Colors.white)
        
        // shadow
        Canvas.print("LIVES %(state.lives)", 180, 1, Colors.greyd)
        
        // forecolor
        Canvas.print("LIVES %(state.lives)", 180, 0, Colors.white)
    }
    
    gameover() {
      Canvas.print("Game Over", (Screen.width.max/2) - 8 * 4.5, Screen.height.max/2, Colors.white)
    }
    
    input() {
        if (!state.isPlaying && Input.x) {
            state.start()
        }
    }
    
    draw() {
        if (state.isPlaying) {
            return scores()
        }
        gameover()
    }
}

class GameState {
    static game {__game}
    static game = (value) {
        __game = value
    }

    static isPlaying {__playing}
    
    static score {
        if (!__score) {
            __score = 0
        }
        
        return __score
    }
    
    static score = (value) {
        __score = value
    }
    
    static lives {
        if (!__lives || __lives < 0) {
            __lives = 3
        }
        
        return __lives
    }
    
    static lives = (value) {
        if (value < 0) {
            over()
        }
        __lives = value
    }
    
    static over() {
        reset()
        __playing = false
    }
    
    static start() {
        __playing = true
    }
    
    static lifeDown() {
        lives = lives - 1
    }
    
    static scoreUp() {
        score = score + 100
    }
    
    static reset() {
        score = 0
        lives = 3
        game.reset()
    }
}

class Breakout {
    player {_player }
    stage {_stage }
    gui {_gui }
    
    construct new() {
        reset()
    }

    reset() {
        GameState.start()
        GameState.game = this

        _player = Player.new(GameState)
        _ball = Ball.new(_player, GameState)
        
        _gui = GUI.new(_player, GameState)
        
        _board = Board.new(_ball, GameState)

        _stage = Stage.new(GameState)
        _stage.add(_player)
        _stage.add(_ball)
        _stage.add(_board)
    }

    input() {
        gui.input()
        stage.input()
    }

    collisions() {
        stage.collisions()
    }

    update() {
        input()
        stage.update()
        collisions()
    }

    draw() {
        Canvas.cls(Colors.black)
        stage.draw()
        gui.draw()
    }
}

class Game {
  static init() {
    Settings.init()
    __game = Breakout.new()
  }

  static update() {
    __game.update()
  }

  static draw(dt) {
    __game.draw()
  }
}
