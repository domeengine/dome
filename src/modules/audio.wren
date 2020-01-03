import "io" for FileSystem
// Represents the data of an audio file
// which can be loaded and unloaded
// It is otherwise opaque Wren-side
foreign class AudioData {
  construct init(buffer) {}
  static fromFile(path) {
    var data = AudioData.init(FileSystem.load(path))
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

class AudioEngine {
  static init() {
    __files = {}
    __channels = {}
    __newChannelId = 0
    f_captureVariable()
  }
  foreign static f_captureVariable()
  // TODO: Allow device enumeration and selection

  // Loading and unloading
  // We only support loading WAV and OGG
  static load(name, path) {
    if (!__files.containsKey(name)) {
      __files[name] = AudioData.fromFile(path)
    }

    return __files[name]
  }
  static unload(name) {
    if (__files.containsKey(name)) {
      __files[name] = null
    }
  }

  static unloadAll() {
    __files = {}
  }

  // audio mix operations
  static play(name) { play(name, 1, false, 0) }
  static play(name, volume) { play(name, volume, false, 0) }
  static play(name, volume, loop) { play(name, volume, loop, 0) }
  static play(name, volume, loop, pan) {
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

  static stopChannel(channelId) {
    if (__channels.containsKey(channelId)) {
      __channels[channelId].enabled = false
    }
  }

  static setChannelVolume(channelId, volume) {
    if (__channels.containsKey(channelId)) {
      __channels[channelId].volume = volume
    }
  }

  static setChannelPan(channelId, pan) {
    if (__channels.containsKey(channelId)) {
      __channels[channelId].pan = pan
    }
  }


  static setChannelLoop(channelId, loop) {
    if (__channels.containsKey(channelId)) {
      __channels[channelId].loop = loop
    }
  }

  static stopAllChannels() {
    __channels.values.each { |channel| channel.enabled = false }
  }

  static isPlaying(channelId) {
    if (__channels.containsKey(channelId)) {
      return !__channels[channelId].isFinished
    }
    return false
  }

  // This is called by DOME
  static update() {
    var playing = __channels.values.where {|channel|
      var stillPlaying = !channel.isFinished
      if (!stillPlaying) {
        __channels.remove(channel.id)
      }
      return stillPlaying
    }.toList
    f_update(playing)
  }
  foreign static f_update(list)
}
AudioEngine.init()
