[< Back](.)

audio
================

The `audio` module lets you play audio files such as music or sound effects.

It contains the following classes:

* [AudioEngine](#audioengine)
* [AudioChannel](#audiochannel)
* [AudioState](#audiostate)

## AudioEngine

DOME only supports OGG and WAV files at the moment.

An audio file is loaded from disk into memory using the `load` function, and remains in memory until you call `unload` or `unloadAll`, or when DOME closes.

When an audio file is about to be played, DOME allocates it an "audio channel", which handles the settings for volume, looping and panning.
Once the audio is stopped or finishes playing, that channel is no longer usable, and a new one will need to be acquired.


### Example

```wren
AudioEngine.load("fire", "res/Laser_Shoot.wav")
var channel = AudioEngine.play("fire")
channel.stop()

...

AudioEngine.unload("fire")
```

### Methods

#### `static isPlaying(channelId: Number): Boolean`
Returns true if the channel _channelId_ is currently playing. A channel cannot restart once it stops playing.
#### `static register(name: String, path: String)`
DOME keeps a mapping from a developer-friendly name to the file path. Calling this method sets up this mapping, but doesn't load that file into memory.

#### `static load(name: String)`
If the `name` has been mapped to a file path, DOME will load that file into memory, ready to play.

#### `static load(name: String, path: String)`
This combines the `register(_,_)` and `load(_)` calls, for convenience.

#### `static play(name: String): AudioChannel`
Plays the named audio sample once, at maximum volume, with equal pan.
#### `static play(name: String, volume: Number): AudioChannel`
Plays the named audio sample once, at _volume_, with equal pan.
#### `static play(name: String, volume: Number, loop: Boolean): AudioChannel`
Plays the named audio sample, at _volume_, with equal pan. If _loop_ is set, the sample will repeat once playback completes.
#### `static play(name: String, volume: Number, loop: Boolean, pan: Number): AudioChannel`
Play the named audio sample and returns the channel object representing that playback.
 * _volume_ - A value with minimum 0.0 for the volume.
 * _loop_ - If true, the audio channel will loop once it is complete.
 * _pan_ - A value between -1.0 and 1.0 which divides the audio playback between left and right stereo channels.

#### `static stopAllChannels()`
Stop all playing audio channels.

#### `static unload(name: String)`
Releases the resources of the chosen audio sample. This will halt any audio using that sample immediately.

## AudioChannel

These are created when you `play` a sample using the AudioEngine. You cannot construct these directly.

### Instance Fields

#### `finished: Boolean`
Returns true if the audio channel has finished playing. It cannot be restarted after this point.

#### `loop: Boolean`
You can set this to control whether the sample will loop once it completes, or stop.
The channel will become invalid if it reaches the end of the sample and `loop` is false.

#### `pan: Number`
This returns a value between -1.0 and 1.0, representing the two-channel pan position. -1.0 is fully-left and 1.0 is fully-right.

You can set this to change the pan position.

#### `position: Number`
This is the current sample position of the audio channel. You cannot change the position.

DOME plays audio at a rate of 44100 samples a second, so to get the position in seconds, you must do `channel.position / 44100`.

#### `soundId: String`
This is the sample name used for this sound.

#### `state: AudioState`
This is an enum which represents the current state of the audio.

#### `volume: Number`
This returns a number with a minimum of 0.0 representing the volume of the channel. 1.0 is the default for the audio data.

You can set this to change the volume.

### Instance Methods

#### `stop(): Void`
Requests that the channel stops as soon as possible.

## AudioState
AudioChannel objects can be in one of the following states:

 - AudioState.INITIALIZE
 - AudioState.TO_PLAY
 - AudioState.PLAYING
 - AudioState.STOPPING
 - AudioState.STOPPED

