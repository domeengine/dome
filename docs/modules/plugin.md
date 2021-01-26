[< Back](.)

plugin
================

The `plugin` module is the gateway to loading [native plugins](/plugins/) which can access lower level system features, or bind to other shared libraries.

## Plugin

### Static Methods

#### `static load(name: String): Void`

This will search the current execution context for a plugin of the given `name` and load it. 

The name of the library must be as follows, for different platforms. Assuming your plugin was named `"test"`:
  * On Windows, the file would be `test.dll`
  * On Mac OS X, the file must be named `test.dylib`
  * On Linux and other platforms, the file must be `test.so`

The `name` can be treated as a relative file path from the location of your application entry point, but it is recommended to place plugin library files in the same folder as your `main.wren` or `game.egg` file, for the best compatibility across platforms.

Once the plugin library is loaded, DOME will execute its [Init hook](/plugins/#init), if available. If the hook reports a failure, or the library could not be found, `Plugin.load()` will abort the current fiber.

A plugin is not unloaded until DOME shuts down. There is no way to unload it during execution.
