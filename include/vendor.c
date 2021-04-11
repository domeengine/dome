#define _DEFAULT_SOURCE
#define NOMINMAX

#ifdef __MINGW32__
#include <windows.h>
#endif

#include <stdbool.h>

#include <jo_gif.h>

#define OPTPARSE_IMPLEMENTATION
#include <optparse.h>

#include <microtar/microtar.c>
#include <json/pdjson.c>
#include <mkdirp/mkdirp.c>

// Set up STB_IMAGE
#define STBI_FAILURE_USERMSG
#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

// Setup STB_VORBIS
#define STB_VORBIS_NO_PUSHDATA_API
#include <stb_vorbis.c>

// Setup ABC_FIFO
#include <SDL.h>
#define ABC_FIFO_IMPL
#include <ABC_fifo.h>


