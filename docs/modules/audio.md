Audio
================

The `audio` module lets you play audio files such as music or sound effects.

It contains the following classes:

* [AudioEngine](#audioengine)

## AudioEngine

The `AudioEngine` is a singleton, whose lifecycle is handled by DOME. Treat it as a class with static methods.

### Methods

#### `isPlaying(channelId)`
#### `load(name, path)`
#### `play(name, volume, loop, pan)`
#### `setChannelLoop(channelId, loop)`
#### `setChannelPan(channelId, pan)`
#### `setChannelVolume(channelId, volume)`
#### `stopChannel(channelId)`
#### `stopAllChannels()`
#### `unload(name)`
#### `unloadAll()`
