import "graphics" for Canvas, Color, Font
import "dome" for Log, Window, Platform, Process
import "Scene" for Scene
import "input" for Mouse
class Main {
  construct new() {
    Window.display = 0
    _leftButtonColor = Color.white
    _rightButtonColor = Color.white
    _fullScreenButtonColor = Color.white
    _quitButtonColor = Color.white
    _buttonPressed = false
  }
  init() {
    
  }
  update() {
    _leftButtonColor = Color.white
    _rightButtonColor = Color.white
    _fullscreenButtonColor = Color.white
    _quitButtonColor = Color.white
    if(Mouse.x>=10 && Mouse.y>=15 && Mouse.x <= 15 && Mouse.y <= 28) {
      _leftButtonColor = Color.red
      if(!_buttonPressed && Mouse.isButtonPressed("left") ) {
        Window.display = Window.display - 1
        _buttonPressed = true
      }
    
    }

    if (Mouse.x>=105 && Mouse.y>=15 && Mouse.x <= 110 && Mouse.y <= 28) {
       _rightButtonColor = Color.red
      if(!_buttonPressed && Mouse.isButtonPressed("left")) {
      
        Window.display = Window.display + 1
        _buttonPressed = true
      }
    }

    if(Mouse.x>=10 && Mouse.y>=30 && Mouse.x <= 100 && Mouse.y <= 38) {
      _fullscreenButtonColor = Color.red
      if(!_buttonPressed && Mouse.isButtonPressed("left")) {
        Window.fullscreen = !Window.fullscreen
        _buttonPressed = true
      }
    }
    if(Mouse.x>=10 && Mouse.y>=40 && Mouse.x <= 60 && Mouse.y <= 48) {
      _quitButtonColor = Color.red
      if(!_buttonPressed && Mouse.isButtonPressed("left")) {
        Process.exit()
        _buttonPressed = true
      }
    }
    if(_buttonPressed && !Mouse.isButtonPressed("left")){
      _buttonPressed = false
    }
  }
  draw(dt) {
    Canvas.cls()
    Canvas.print("<", 10, 20, _leftButtonColor)
    Canvas.print("Display: %(Window.display)", 20, 20, Color.white)
    Canvas.print(">", 105, 20, _rightButtonColor)
    Canvas.print("Fullscreen", 20, 30, _fullscreenButtonColor)
    Canvas.print("Quit", 20, 40, _quitButtonColor)
  }
}

var Game = Main.new()
