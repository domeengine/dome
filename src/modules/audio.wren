// Represents the data of an audio file
// which can be loaded and unloaded
// It is otherwise opaque Wren-side

/*
musicChannel.play()
musicChannel.play(FadeInEffect(time))
musicChannel.stop(FadeOutEffect(time))
musicChannel.pause(FadeInEffect(time), FadeOutEffect(time))

musicChannel.commit__()

if (musicChannel.stopped) {
  // There's concerns about releasing audio resources
  // because it could break the mixer.
  // Probably going to be a slow operation
  AudioEngine.unload("music")
}

// ----------------- EXAMPLE CODE
*/

foreign class AudioData {
  construct init(buffer) {}
  static loadFromFile(path) {
    import "io" for FileSystem
    var data = AudioData.init(FileSystem.load(path))
    System.print("Audio loaded: " + path)
    return data
  }
  foreign length
}

// Base interface for audio channels
class AudioChannel {}

class AudioState {
  static INITIALIZE { 1 }
  static TO_PLAY { 2 }
  static DEVIRTUALIZE { 3 }
  static LOADING { 4 }
  static PLAYING { 5 }
  static STOPPING { 6 }
  static STOPPED { 7 }
  static VIRTUALIZING { 8 }
  static VIRTUAL { 9 }
}

// Encapsulates the data of the currently playing channel
foreign class SystemChannel is AudioChannel {
  construct new(soundId) {}
  foreign audio=(value)


  foreign soundId
  foreign position

  foreign state
  foreign state=(value)

  foreign enabled=(enable)
  foreign enabled

  foreign loop=(do)
  foreign loop

  foreign pan=(pan)
  foreign pan

  foreign volume=(volume)
  foreign volume
}

class AudioChannelFacade is AudioChannel {
  construct wrap(id, channel) {
    _channel = channel
    _volume = 1
    _pan = 0
    _loop = false
    _stopRequested = false
    _id = id
  }

  stop() {
    _stopRequested = true
  }

  update_() {
    if (state == AudioState.INITIALIZE) {
      _channel.state = AudioState.TO_PLAY
    }

    if (state == AudioState.TO_PLAY || state == AudioState.DEVIRTUALIZE) {
      // Assume Data is loaded by this point
      commit_()
      _channel.state = AudioState.PLAYING
      _channel.enabled = true
      return
    }

    if (state == AudioState.PLAYING) {
      commit_()
      if (finished || _stopRequested) {
        _channel.state = AudioState.STOPPING
      }
      return
    }

    if (state == AudioState.STOPPING) {
      // TODO: Fade
      commit_()
      // if fade complete
      _channel.enabled = false
      if (!_channel.enabled) {
        _channel.state = AudioState.STOPPED
      }
      return
    }

    if (state == AudioState.STOPPING) {}
  }

  commit_() {
    _channel.volume = _volume
    _channel.pan = _pan
    _channel.loop = _loop
  }

  // Private
  channel_ { _channel }
  id { _id }

  // Public
  position { _channel.position }
  soundId { _channel.soundId }
  volume { _volume }
  volume=(volume) { _volume = volume }
  loop { _loop }
  loop=(loop) { _loop = loop }
  pan { _pan }
  pan=(pan) { _pan = pan }
  state { _channel.state }
  finished { !_channel.enabled || state == AudioState.STOPPED }

}

class AudioEngine {
  // TODO: Allow device enumeration and selection
  static init() {
    __unloadQueue = []
    __nameMap = {}
    __files = {}
    __nextId = 0
    __channels = {}
    f_captureVariable()
  }
  foreign static f_captureVariable()

  static register(name, path) {
    __nameMap[name] = path
  }
  static load(name, path) {
    register(name, path)
    return load(name)
  }

  static load(name) {
    if (!__nameMap.containsKey(name)) {
      Fiber.abort("Audio '%(name)' has not been registered ")
    }
    var path = __nameMap[name]
    if (!__files.containsKey(path)) {
      __files[path] = AudioData.loadFromFile(path)
    }
    return __files[path]
  }

  static unload(name) {
    __unloadQueue.add(name)
  }

  static play(name) { play(name, 1, false, 0) }
  static play(name, volume) { play(name, volume, false, 0) }
  static play(name, volume, loop) { play(name, volume, loop, 0) }
  static play(name, volume, loop, pan) {
    var systemChannel = SystemChannel.new(name)
    systemChannel.audio = load(name)
    var channel = AudioChannelFacade.wrap(__nextId, systemChannel)
    __channels[__nextId] = channel
    channel.volume = volume
    channel.pan = pan
    channel.loop = loop
    // channel.commit_()

    __nextId = __nextId + 1
    return channel
  }

  static stopAllChannels() {
    __channels.values.each { |channel| channel.stop() }
  }

  foreign static f_update(list)
  static update() {
    var playing = __channels.values.where {|facade|
      if (__unloadQueue.contains(facade.soundId)) {
        __channels.remove(facade.id)
        return false
      }
      facade.update_()
      if (facade.state == AudioState.STOPPED) {
        __channels.remove(facade.id)
        return false
      }
      return facade.state == AudioState.PLAYING ||
             facade.state == AudioState.STOPPING ||
             facade.state == AudioState.VIRTUALIZING
    }.map {|facade| facade.channel_ }.toList

    f_update(playing)

    if (__unloadQueue.count > 0) {
      __unloadQueue.each {|soundId|
        if (__nameMap.containsKey(soundId)) {
          var path = __nameMap[soundId]
          if (__files.containsKey(path)) {
            __files.remove(path)
          }
        }
      }
      // We have to force a gc here to release audio objects.
      System.gc()
      __unloadQueue = []
    }
  }

}
AudioEngine.init()
