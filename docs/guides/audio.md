[< Back](..)

How to play sounds?
==========================

DOME supports playback of audio files in OGG, MP3, FLAC and WAV formats. It will convert
all files to its native sample rate of 44.1kHz (CD quality audio), but the
re-sampling algorithm used is naive and may introduce audio artifacts. It is
recommended that you produce your audio with a 44.1kHz sample-rate, for the
best quality audio.

You can access most of DOME's audio functions through the
[`AudioEngine`](/modules/audio#audioengine) class in the
[`audio`](/modules/audio) module.

## Loading

Before you can play a file, you need to load it into the AudioEngine. Files are
loaded and assigned a label, for future playback. After that, the audio is playable.

```
import "audio" for AudioEngine
AudioEngine.load("cool-sounds", "path/to/file")
```

Now, you can use the AudioEngine to play the `"cool-sounds"` audio asset.

## Playing


When an audio file is played, it plays in its own "channel". All the channels
are stereo, and mixed together by DOME based on volume and pan settings.
A channel can also be stopped early. Channels can repeat, but once stopped,
they are no longer usable. You need to play the sound again to create a new
channel.

Here is a simple example of playing a sound, and configuring the volume and pan settings:
```
import "audio" for AudioEngine

AudioEngine.load("cool-sounds", "path/to/file")
var channel = AudioEngine.play("cool-sounds")
channel.volume = 0.5 // Play at half volume
channel.pan = 0.3 // Position towards the right
```




