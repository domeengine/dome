[< Back](.)

Importing User Modules
===================

Wren allows scripts to import modules of reusable functionality specific to the embedding environment.

In our case, DOME allows for [built-in modules](../modules) to be imported by name like this:

```
import "[module_name]" for ClassName
```

However, imagine you wanted to import a class called `Map` from a custom module in a file called `map.wren` in the same directory as your current game. You would import it like this:

```
import "[path]" for ClassName
```

DOME currently resolves paths in a very simple way: All are relative to the entry point of the game, which is usually `main.wren`, and must begin with `./`.
  
As an example, imagine this directory structure:
```
.
+-- dome
+-- main.wren
+-- map.wren
+-- objects
    +-- sprite.wren
    +-- background.wren
+-- utils
    +-- math.wren
    +-- vectors.wren
```

To import a class from `utils/math.wren` from `objects/sprite.wren`, you would have an import statement like this:
```
import "./utils/math" for Math
```
This is because even though `sprite.wren` is in the `objects` folder, the path has to be relative to the `main.wren` of the project.


