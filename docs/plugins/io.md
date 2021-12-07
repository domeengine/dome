[< Back](.)

I/O
===============

This set of APIs allows you to access the host filesystem to read files.

  * [Acquisition](#acquistion)
  * [method: readfile](#method-readfile)

## Acquisition

```c
IO_API_v0* io = (IO_API_v0*)DOME_getAPI(API_IO, IO_API_VERSION);
```

## Methods

### method: readFile
```c
void* readFile(DOME_Context ctx, const char* path, size_t* length);
```
Synchronously reads the file located at `path` to memory. The size of the file 
in bytes is stored in the location pointed to by `length`. You are responsible 
for freeing the returned pointer when you are done using it.

If there is a problem loading the file, this method will return `NULL`. You can 
find out why using `core->getLastError(DOME_Context ctx)`;
