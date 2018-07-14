// Encapsulates the data of the currently playing channel
class Channel {
  construct new(id, audio) {}
}

// Represents the data of an audio file
// which can be loaded and unloaded
// It is otherwise opaque Wren-side
foreign class AudioData {
  construct fromFile(path) {}
  foreign unload()
}

foreign class AudioEngineImpl {
  construct init() {
    __files = {}
  }
  // TODO: Allow device enumeration and selection

  // Loading and unloading
  // We only support loading WAV and (maybe) OGG
  load(name, path) {
    if (!__files.containsKey(name)) {
      __files[name] = AudioData.fromFile(path)
    }

    return __files[name]
  }
  unload(name) {
    if (__files.containsKey(name)) {
      __files[name] = null
    }
  }
  unloadAll() {
    __files = {}
  }

  // audio mix operations
  play(name) { play(name, 0.5, 0) }
  play(name, volume) {}
  play(name, volume, pan) {}

  stopChannel(channelId) {}
  setChannelVolume(channelId, volume) {}
  setChannelPan(channelId, pan) {}
  stopAllChannels() {}
  isPlaying(channelId) {}

  foreign update()
}

// We only intend to expose this
var AudioEngine = AudioEngineImpl.init()
