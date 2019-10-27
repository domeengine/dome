import "io" for FileSystem
// Represents the data of an audio file
// which can be loaded and unloaded
// It is otherwise opaque Wren-side
foreign class AudioData {
  construct init(buffer) {}
  static fromFile(path) {
    var data = AudioData.init(FileSystem.loadSync(path))
    System.print("Audio loaded: " + path)
    return data
  }
  foreign unload()
}

// Encapsulates the data of the currently playing channel
foreign class AudioChannel {
  construct new(id, audio) {}
  foreign isFinished
  foreign id
  foreign enabled=(id)
  foreign loop=(do)
  foreign pan=(pan)
  foreign volume=(volume)
}

foreign class AudioEngineImpl {
  construct init() {
    __files = {}
    __channels = {}
    __newChannelId = 0
  }
  // TODO: Allow device enumeration and selection

  // Loading and unloading
  // We only support loading WAV and OGG
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
  play(name) { play(name, 1, false, 0) }
  play(name, volume) { play(name, volume, false, 0) }
  play(name, volume, loop) { play(name, volume, loop, 0) }
  play(name, volume, loop, pan) {
    if (__files.containsKey(name)) {
      __newChannelId = __newChannelId + 1
      var channel = AudioChannel.new(__newChannelId, __files[name])
      __channels[__newChannelId] = channel
      channel.loop = loop
      channel.volume = volume
      channel.pan = pan
    }

    return __newChannelId
  }

  stopChannel(channelId) {
    if (__channels.containsKey(channelId)) {
      __channels[channelId].enabled = false
    }
  }

  setChannelVolume(channelId, volume) {
    if (__channels.containsKey(channelId)) {
      __channels[channelId].volume = volume
    }
  }

  setChannelPan(channelId, pan) {
    if (__channels.containsKey(channelId)) {
      __channels[channelId].pan = pan
    }
  }


  setChannelLoop(channelId, loop) {
    if (__channels.containsKey(channelId)) {
      __channels[channelId].loop = loop
    }
  }

  stopAllChannels() {
    __channels.values.each { |channel| channel.enabled = false }
  }

  isPlaying(channelId) {
    if (__channels.containsKey(channelId)) {
      return !__channels[channelId].isFinished
    }
    return false
  }

  update() {
    var playing = __channels.values.where {|channel|
      var stillPlaying = !channel.isFinished
      if (!stillPlaying) {
        __channels.remove(channel.id)
      }
      return stillPlaying
    }.toList
    f_update(playing)
  }
  foreign f_update(list)
}

// We only intend to expose this
var AudioEngine = AudioEngineImpl.init()

// We need the same engine under a different name so that we can
// call it from C with a unique import.
var AudioEngine_internal = AudioEngine
