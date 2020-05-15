[< Back](..)

Importing User Modules
===================

Wren allows scripts to import modules of reusable functionality specific to the embedding environment.

In our case, DOME allows for modules to be imported by path like this:

```
import "[module_path]" for ClassName
```

DOME currently resolves paths in a very simple way: All are relative to the entry point of the game, which is usually `main.wren`. 

Module paths are resolved with the following priority:
* DOME's [built-in modules](../modules)
* Wren VM built-in modules `random` and `meta`.
* User-provided modules at the specified path, relative to the game entry point
  
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


