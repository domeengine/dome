[< Back](.)

Audio
===============

This set of APIs gives you access to DOME's audio engine, to provide your own audio channel implementations. You can use this to synthesize sounds, or play custom audio formats.

 * [Acquisition](#acquistion)
 * Enums
   - [enum: CHANNEL_STATE](#enum-channel_state)
 * Function Signatures
   - [function: CHANNEL_mix](#function-channel_mix)
   - [function: CHANNEL_callback](#function-channel_callback)
 * Methods
   - [method: channelCreate](#method-channelcreate)
   - [method: getData](#method-getdata)
   - [method: getState](#method-getstate)
   - [method: setState](#method-setstate)
   - [method: stop](#method-stop)


## Acquisition

```c
AUDIO_API_v0* audio = (AUDIO_API_v0*)DOME_getAPI(API_AUDIO, AUDIO_API_VERSION);
```

## Enums

### enum: CHANNEL_STATE

Audio channels are enabled and disabled based on a state, which is represented by this enum. Supported states are the following:

```c
enum CHANNEL_STATE {
  CHANNEL_INITIALIZE,
  CHANNEL_TO_PLAY,
  CHANNEL_PLAYING,
  CHANNEL_STOPPING,
  CHANNEL_STOPPED
}
```

## Function Signatures

### function: CHANNEL_mix
`CHANNEL_mix` functions have a signature of `void mix(CHANNEL_REF ref, float* buffer, size_t sampleRequestSize)`. 

  * `ref` is a reference to the channel being mixed. 
  * `buffer` is an interleaved stereo buffer to write your audio data into. One sample is two values, for left and right, so `buffer` is `2 * sampleRequestSize` in size. 

This callback is called on DOME's Audio Engine mixer thread. It is essential that you avoid any slow operations (memory allocation, network) or you risk interruptions to the audio playback.

### function: CHANNEL_callback
`CHANNEL_callback` functions have this signature: `void callback(CHANNEL_REF ref, WrenVM* vm)`.


## Methods

### method: channelCreate
```c
CHANNEL_REF channelCreate(DOME_Context ctx,
                          CHANNEL_mix mix, 
                          CHANNEL_callback update, 
                          CHANNEL_callback finish, 
                          void* userdata);
```

When you create a new audio channel, you must supply callbacks for mixing, updating and finalizing the channel. This allows it to play nicely within DOME's expected audio lifecycle.

This method creates a channel with the specified callbacks and returns its corresponding CHANNEL_REF value, which can be used to manipulate the channel's state during execution. The channel will be created in the state `CHANNEL_INITIALIZE`, which gives you the opportunity to set up the channel configuration before it is played.

The callbacks work like this:
  - `update` is called once a frame, and can be used for safely modifying the state of the channel data. This callback holds a lock over the mixer thread, so avoid holding it for too long.
  - `finish` is called once the channel has been set to `STOPPED`, before its memory is released. It is safe to expect that the channel will not be played again.

The `userdata` is a pointer set by the plugin developer, which can be used to pass through associated data, and retrieved by [`getData(ref)`](#method-getdata). You are responsible for the management of the memory pointed to by that pointer and should avoid modifying the contents of the memory outside of the provided callbacks.


### method: getData
```c
void* getData(CHANNEL_REF ref)
```
Fetch the `userdata` pointer for the given channel `ref`.

### method: getState
```c
CHANNEL_STATE getState(CHANNEL_REF ref)
```
Get the current [state](#enum-channel_state) of the channel specified by `ref`.

### method: setState
```c 
void setState(CHANNEL_REF ref, CHANNEL_STATE state)
```
This allows you to specify the channel's [state](#enum-channel_state). DOME will only mix in channels in the following states: `CHANNEL_PLAYING` and `CHANNEL_STOPPING`.

### method: stop
```c
void stop(CHANNEL_REF ref)
```
Marks the audio channel as having stopped. This means that DOME will no longer play this channel. It will call the `finish` callback at it's next opportunity.
