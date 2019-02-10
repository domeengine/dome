audio
================

The `audio` module lets you play audio files such as music or sound effects.

It contains the following classes:

* [AudioEngine](#audioengine)

## AudioEngine

The `AudioEngine` is a singleton, whose lifecycle is partially managed by DOME. Treat it as a class with static methods.

DOME currently supports OGG and WAV files, and can play up to four audio channels simultaneously. 

An audio file is loaded from disk into memory using the `load` function, and remains in memory until you call `unload` or `unloadAll`, or when DOME closes.

### Example

```wren
AudioEngine.load("fire", "res/Laser_Shoot.wav")
var id = AudioEngine.play("fire")
AudioEngine.stopChannel(id)

...

AudioEngine.unload("fire")
```

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
