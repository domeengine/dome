// title: snake clone
// author: camilo castro
// desc: A small snake game clone in wren
// script: wren
// based on: https://github.com/nesbox/TIC-80/wiki/Snake-Clone-tutorial
// original: https://github.com/NinjasCL/snake
// license: mit

import "random" for Random
import "graphics" for Color, Canvas
import "input" for Keyboard
import "dome" for Window

var Rand = Random.new()

class Input {
		static up {Keyboard.isKeyDown("up")}
		static down {Keyboard.isKeyDown("down")}
		static left {Keyboard.isKeyDown("left")}
		static right {Keyboard.isKeyDown("right")}
		static x {Keyboard.isKeyDown("x")}
}

class Screen {
    static width {240}
		static height {136}
    static scale {4}
    static title {"🐍 Snake"}

    static init() {
      Canvas.resize(Screen.width, Screen.height)
      Window.resize(scale * Canvas.width, scale * Canvas.height)
      Window.title = title
    }
}

class Position {
  x {
		if (!_x) {
			_x = 0
			}
			return _x
		}
		
		x = (value) {
		_x = value
		}
		
		y {
		if (!_y) {
			_y = 0
			}
			return _y
		}
		
		y = (value) {
		_y = value
		}
		
		construct new(x, y) {
			_x = x
			_y = y
		}
}

class GameState {
  static game {__game}
		static game = (value) {
			__game = value
		}
		
		static snake {__snake}
		static snake = (value) {
			__snake = value
		}
		
		static score {
			if (!__score) {
				__score = 0
			}
			return __score
		} 
		
		static scoreUp() {
			__score = score + 100
		}
		
  static frame {
		if (!__frame || __frame > Num.largest - 1) {
					__frame = 0
				}
				return __frame
		}
		
		static frameUp() {
			__frame = frame + 1
		}
		
		static isTenthFrame {frame % 10 == 0}
		
		static isPlaying {
			if (__isPlaying is Null) {
				__isPlaying = true
			}
			return __isPlaying
		}
		
		static isPlaying = (value) {
			__isPlaying = value
		}
		
		static start() {
			isPlaying = true
		}
		
		static gameover() {
			reset()
				isPlaying = false
		}
		
		static reset() {
			__score = 0
			game.reset()
		} 
}

class Food is Position {
	snake {_snake}
	
	construct new (snake) {
			_snake = snake
			place()
	}
	
	random() {
		x = Rand.int(0, 29)
		y = Rand.int(0, 16)
	}
	
	place() {
		random()
		snake.body.each{|part|
			if (part.x == x && part.y == y) {
					place()
			}
		}
	}
	
	draw() {
	  Canvas.rectfill(x * 8, y * 8, 8, 8, Color.orange)
	}
	
	update() {}
	input() {}
	
	collisions() {
		snake.eat(this)
	}
}

class Direction {
	static directions {
		if (!__directions) {
				__directions = [
						Position.new(0, -1), // up
						Position.new(0, 1), // down
						Position.new(-1, 0), // left
						Position.new(1, 0) // right
				]
			}
			return __directions
		}
		
		static up {directions[0]}
		static down {directions[1]}
		static left {directions[2]}
		static right {directions[3]}
}

class Snake {
  tail {body[0]}
		neck {body[body.count - 2]}
		head {body[body.count - 1]}
		body {
			if (!_body) {
					var tail = Position.new(15, 8)
					var neck = Position.new(14, 8)
					var head = Position.new(13, 8)
					_body = [tail, neck, head]
				}
				return _body
		}
		
		direction {
			if (!_direction) {
				_direction = Direction.up
			}
			return _direction
		}
		
		state {_state}
		
		construct new(state) {
			_state = state
		}
		
		reset() {
			_body = null
			_direction = Direction.up
		}
		
		input() {
			var last = direction
			
			if (Input.up) {
				_direction = Direction.up
			}
			
			if (Input.down) {
				_direction = Direction.down
			}
			
			if (Input.left) {
				_direction = Direction.left
			}
			
			if (Input.right) {
				_direction = Direction.right
			}
			
			if (head.x + direction.x == neck.x &&
				head.y + direction.y == neck.y) {
					_direction = last
			}
		}
		
		draw() {
			body.each{|part|
			 Canvas.rectfill(part.x * 8, part.y * 8, 8, 8, Color.green)
			}
		}
		
		update() {
		
			var x = (head.x + direction.x) % 30
			if (x < 0) {
				x = 29
			}
			
			var y = (head.y + direction.y) % 17
			if (y < 0) {
				y = 17
			}
			
			var part = Position.new(x, y)
			
			body.add(part)
		}
		
		removeTail() {
			body.removeAt(0)
		}
		
		eat(food) {
			if (head.x == food.x && head.y == food.y) {
				state.scoreUp()
				food.place()
			} else {
				removeTail()
			}
		}
		
		collisions() {
				
				// Search if head collisions with body parts
				// omit last part since its the head
			body[0...(body.count - 1)].each{|part|
					if (head.x == part.x && head.y == part.y) {
							state.gameover()
					}
			}
		}
}

class Stage {
  items {_items}
		
	construct new() {
		_items = []
	}
		
  add(object) {
			items.add(object)
	}
		
  input() {
			items.each{|object|
					object.input()
			}
	}
		
	draw() {
		items.each{|object|
			object.draw()
		}
	}
	
	update() {
		items.each{|object|
			object.update()
		}
	}
	
	collisions() {
		items.each{|object|
			object.collisions()
		}
	}
}

class GUI {
		state {_state}
		
		construct new(state) {
			_state = state
		}
		
		score() {
			Canvas.print("SCORE %(state.score)", 5, 5, Color.blue)
			Canvas.print("SCORE %(state.score)", 5, 4, Color.white)
		}
		
		gameover() {
			Canvas.cls(Color.black)
			Canvas.print("Game Over", (Screen.width/2) - 6 * 4.5, (Screen.height / 2), Color.white)
		}
		
		input() {
			if (!state.isPlaying && Input.x) {
					state.start()
			}
		}
		
		update() {}
		collisions() {}
		
		draw() {
			if (state.isPlaying) {
					return score()
			}
			gameover()
		}
}

class SnakeGame {
		stage {
			if (!_stage) {
					GameState.game = this
					_stage = Stage.new()
					_stage.add(snake)
					_stage.add(food)
					_stage.add(gui)
			}
			return _stage
		}
		
		gui {
			if (!_gui) {
				_gui = GUI.new(GameState)
			}
			return _gui
		}
		
		snake {
			if (!_snake) {
				_snake = Snake.new(GameState)
				GameState.snake = _snake
			}
			return _snake
		}
		
		food {
			if (!_food) {
				_food = Food.new(snake)
			}
			return _food
		}
		
		construct new() {
				reset()
		}
		
		reset() {
			snake.reset()
		}
		
		input() {
			stage.input()
		}
		
		update() {
			stage.update()
		}
		
		collisions() {
			stage.collisions()
		}
		
		draw() {
			stage.draw()
		}
}

class Game {
  static init() {
    Screen.init()
    __game = SnakeGame.new()
  }
  
  static update() {
			Canvas.cls(Color.peach)
			GameState.frameUp()
			__game.input()
			
			if (GameState.isTenthFrame) {
				__game.update()
				__game.collisions()
			}
  }

  static draw(dt) {
    __game.draw()
  }
}
