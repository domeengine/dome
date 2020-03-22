[< Back](.)

audio
================

The `audio` module lets you play audio files such as music or sound effects.

It contains the following classes:

* [AudioEngine](#audioengine)
* [AudioChannel](#audiochannel)
* [AudioState](#audiostate)

## AudioEngine

At the moment, DOME only supports OGG and WAV files, with a sample frequency of 44.1kHz (CD quality audio)

An audio file is loaded from disk into memory using the `load` function, and remains in memory until you call `unload(_)` or `unloadAll()`, or when DOME closes.

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
The other parameters are explained in the [AudioChannel](#audiochannel) api.

#### `static stopAllChannels()`
Stop all playing audio channels.

#### `static unloadAll()`
Releases the resources of the all currently loaded audio samples. This will halt any audio using that sample immediately.

#### `static unload(name: String)`
Releases the resources of the chosen audio sample. This will halt any audio using that sample immediately.

## AudioChannel

These are created when you `play` a buffer of AudioData using the AudioEngine. You cannot construct these directly. 

A playing audio channel has three main properties:
 * _volume_ - A value with minimum 0.0 for the volume.
 * _loop_ - If true, the audio channel will loop once it is complete.
 * _pan_ - A value between -1.0 and 1.0 which divides the audio playback between left and right stereo channels.

### Instance Fields

#### `finished: Boolean`
Returns true if the audio channel has finished playing. It cannot be restarted after this point.

#### `length: Number`
The total number of samples in this channel's audio buffer.
You should divide this by `44100` to get the length in seconds.

#### `loop: Boolean`
You can set this to control whether the sample will loop once it completes, or stop.
The channel will become invalid if it reaches the end of the sample and `loop` is false.

#### `pan: Number`
You can read and modify the pan position, as a bounded value between -1.0 and 1.0.

#### `position: Number`
This marks the position of the next sample to be loaded into the AudioEngine mix buffer (which happens on a seperate thread).
You can set a new position for the channel, but it isn't going to be exact, due to delays in audio processing.

You should divide this by `44100` to get the position in seconds.

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

