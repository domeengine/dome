[< Back](.)

Getting Started
=================

To get started, make sure you've acquired DOME using the [installation](installation) instructions.

Place the `dome` executable to a directory which will serve as your workspace.

Next, create a new file in your workspace directory named `main.wren`. DOME looks by default for a `main.wren` or `game.egg` file (see [Distributing your Game](guides/distribution)) as the entry point to your game.

Open `main.wren` in your favourite text editor and enter the following:

```
class Game {
  static init() {}
  static update() {}
  static draw(alpha) {}
}
```

Save the file and navigate to your workspace in the terminal. Run `./dome` and you should see a black box appear.
Now you're ready to make your first game!

