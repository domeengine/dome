[< Back](.)

audio
================

The `audio` module lets you play audio files such as music or sound effects.

It contains the following classes:

* [AudioEngine](#audioengine)

## AudioEngine

DOME only supports OGG and WAV files at the moment.

An audio file is loaded from disk into memory using the `load` function, and remains in memory until you call `unload` or `unloadAll`, but all audio data is also unloaded when DOME closes.

The API for DOME's audio engine is heavily influenced by [this talk](https://www.youtube.com/watch?v=Vjm--AqG04Y) by Guy Somberg.

### Example

```wren
AudioEngine.load("fire", "res/Laser_Shoot.wav")
var id = AudioEngine.play("fire")
AudioEngine.stopChannel(id)

...

AudioEngine.unload("fire")
```

### Methods

#### `static isPlaying(channelId: Number): Boolean`
Returns true if the channel _channelId_ is currently playing. A channel cannot restart once it stops playing.
#### `static load(name: String, path: String)`
Load the audio file from the specified _path_ and assign it a _name_ for future playback.

#### `static play(name: String): Number`
Plays the named audio sample once, at maximum volume, with equal pan.
#### `static play(name: String, volume: Number): Number`
Plays the named audio sample once, at _volume_, with equal pan.
#### `static play(name: String, volume: Number, loop: Boolean): Number`
Plays the named audio sample, at _volume_, with equal pan. If _loop_ is set, the sample will repeat once playback completes.
#### `static play(name: String, volume: Number, loop: Boolean, pan: Number): Number`
Play the named audio sample and return the channel id is plays on.
 * _volume_ - A value with minimum 0.0 for the volume.
 * _loop_ - If true, the audio channel will loop once it is complete.
 * _pan_ - A value between -1.0 and 1.0 which divides the audio playback between left and right stereo channels.

#### `static setChannelLoop(channelId: Number, loop: Boolean)`
If true, the channel will loop once playback completes.

#### `static setChannelPan(channelId: Number, pan: Number)`
Pan divides the audio playback between left and right stereo channels, as a value of -1.0 to 1.0

#### `static setChannelVolume(channelId: Number, volume: Number)`
Set the volume of the channel between 0.0 and 1.0.

#### `static stopChannel(channelId: Number)`
If it is playing, stop the chosen audio channel.

#### `static stopAllChannels()`
Stop all playing audio channels.

#### `static unload(name: String)`
Releases the resources of the chosen audio sample.

#### `static unloadAll()`
Release all audio samples.
