typedef struct {
  bool solid;
  // door state
  bool door;
  bool locked;
  int behaviour; // how does this door function?
  double state; // how open are we, clamped [0,1]
  int8_t mode; // opening/closing

  bool thin;
  double offset;

  // If these are negative, default to color
  int wallTextureId;
  int floorTextureId;
  int ceilingTextureId;
} TILE;


typedef struct {
  size_t width;
  size_t height;
  TILE* tiles;
} MAP;

typedef struct {
  uint64_t id;
  V2 position;
  V2 direction;
  int textureId;
  V2 div;
  double vMove;
} OBJ;

typedef struct {
  uint64_t id;
  WrenHandle* handle;
} OBJ_REF;

typedef struct {
  V2 position;
  V2 direction;
  V2 cameraPlane;

  uint32_t width;
  uint32_t height;

  double* lookup;
  double* z;

  DOME_Bitmap** textureList;

  size_t objectCount;
  OBJ* objects;

  MAP map;
  uint64_t nextId;
} RENDERER;

typedef struct {
  size_t x;
  size_t y;
  WrenHandle* handle;
} TILE_REF;

typedef struct {
  V2 mapPos;
  int side;
  V2i stepDirection;
  bool inBounds;
} CAST_RESULT;

static CANVAS_API_v0* canvas;
static BITMAP_API_v0* bitmap;
static DOME_API_v0* core;
static IO_API_v0* io;
static WREN_API_v0* wren;
static void (*unsafePset)(DOME_Context, int32_t, int32_t, DOME_Color) = NULL;
static WrenVM* vm;
static TILE VOID_TILE = {};
