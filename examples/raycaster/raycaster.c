#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include "math.h"
#include "domath.c"
#include "sbuf.c"

// You'll need to include the DOME header
#include "dome.h"
#include "renderer.c.inc"

static GRAPHICS_API_v0* graphics;
static DOME_API_v0* core;
static IO_API_v0* io;
static WREN_API_v0* wren;
static void (*unsafePset)(DOME_Context, int32_t, int32_t, DOME_Color) = NULL;

static uint32_t WIDTH = 0;
static uint32_t HEIGHT = 0;

typedef struct {
  bool solid;
  // door state
  bool door;
  bool locked;
  int behaviour; // how does this door function?
  double state; // how open are we, clamped [0,1]
  int8_t mode; // opening/closing

  double offset; // if -1, it's not thin

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
  MAP map;
  DOME_Bitmap** textureList;
} RENDERER;

typedef struct {
  size_t x;
  size_t y;
  RENDERER* renderer; // Should this be a wren handle?
} TILE_REF;

#define getTileFrom_fast(ref, renderer) renderer->map.tiles[ref->y * renderer->map.width + ref->x]
#define getTileFrom(ref) getTileFrom_fast(ref, ref->renderer)

uint32_t BLACK = 0xFF000000;
uint32_t R =  0xFFFF0000;
uint32_t B = 0xFF0000FF;
uint32_t texture[64] = {};

#define mapWidth 24
#define mapHeight 24

int worldMap[mapHeight][mapWidth]=
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,6,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

double posX = 22, posY = 12;  //x and y start position
double dirX = -1, dirY = 0; //initial direction vector
double planeX = 0, planeY = -1; //the 2d raycaster version of camera plane

void TILE_allocate(WrenVM* vm) {
  TILE_REF* ref = wren->setSlotNewForeign(vm, 0, 0, sizeof(TILE_REF));
  RENDERER* renderer = wren->getSlotForeign(vm, 1);
  ref->x = wren->getSlotDouble(vm, 2);
  ref->y = wren->getSlotDouble(vm, 3);
  ref->renderer = renderer;
}

void TILE_setTextures(WrenVM* vm) {
  TILE* tile = wren->getSlotForeign(vm, 0);
  size_t arity = wren->getSlotCount(vm);
  tile->wallTextureId = wren->getSlotDouble(vm, 1);
  if (arity >= 3) {
    tile->floorTextureId = wren->getSlotDouble(vm, 2);
  }
  if (arity >= 4) {
    tile->ceilingTextureId = wren->getSlotDouble(vm, 3);
  }
}

#define TILE_GETTER(fieldName, method, fieldType) \
  void TILE_get##method(WrenVM* vm) { \
  TILE_REF* ref = wren->getSlotForeign(vm, 0); \
  RENDERER* renderer = ref->renderer; \
  TILE tile = getTileFrom(ref); \
  wren->setSlot##fieldType(vm, 0, tile.fieldName); \
}

#define TILE_SETTER(fieldName, method, fieldType) \
void TILE_set##method(WrenVM* vm) { \
  TILE_REF* ref = wren->getSlotForeign(vm, 0); \
  RENDERER* renderer = ref->renderer; \
  getTileFrom(ref).fieldName = wren->getSlot##fieldType(vm, 1); \
}

TILE_GETTER(solid, Solid, Bool)
TILE_SETTER(solid, Solid, Bool)
TILE_GETTER(door, Door, Bool)
TILE_SETTER(door, Door, Bool)
TILE_GETTER(state, State, Double)
TILE_SETTER(state, State, Double)
TILE_GETTER(mode, Mode, Double)
TILE_SETTER(mode, Mode, Double)

void allocate(WrenVM* vm) {
  DOME_Context ctx = core->getContext(vm);
  WIDTH = graphics->getWidth(ctx);
  HEIGHT = graphics->getHeight(ctx);
  size_t CLASS_SIZE = sizeof(RENDERER); // This should be the size of your object's data
  RENDERER* obj = wren->setSlotNewForeign(vm, 0, 0, CLASS_SIZE);
  obj->textureList = NULL;
  obj->map.tiles = NULL;
  obj->map.width = 0;
  obj->map.height = 0;

  // Temporary map loading
  obj->map.tiles = NULL;
  obj->map.width = mapHeight;
  obj->map.height = mapWidth;
  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      TILE tile;
      tile.solid = worldMap[y][x] != 0;
      tile.wallTextureId = worldMap[y][x];
      tile.floorTextureId = 0;
      tile.ceilingTextureId = 0;
      tile.door = worldMap[y][x] == 6;
      tile.state = tile.door ? 0.5 : 0;
      if (tile.door) {
        printf("tile.door = 6\n");
      }
      tile.locked = false;
      sbpush(obj->map.tiles, tile);
    }
  }
}

void finalize(void* data) {
  RENDERER* renderer = data;
  for (int i = 0; i < sbcount(renderer->textureList); i++) {
    io->freeBitmap(renderer->textureList[i]);
  }
  sbfree(renderer->textureList);
  sbfree(renderer->map.tiles);
}

void loadTexture(WrenVM* vm) {
  DOME_Context ctx = core->getContext(vm);
  RENDERER* renderer = wren->getSlotForeign(vm, 0);
  const char* path = wren->getSlotString(vm, 1);
  size_t newId = sbcount(renderer->textureList);
  DOME_Bitmap* bitmap = io->readImageFile(ctx, path);
  sbpush(renderer->textureList, bitmap);
  wren->setSlotDouble(vm, 0, newId);
  printf("Assigning texture slot %zu\n", newId);
}

void setPosition(WrenVM* vm) {
  double x = wren->getSlotDouble(vm, 1);
  double y = wren->getSlotDouble(vm, 2);
  posX = x;
  posY = y;
}
void setAngle(WrenVM* vm) {
  double angle = wren->getSlotDouble(vm, 1);
  double rads = angle * M_PI / 180.0;
  dirX = cos(rads);
  dirY = sin(rads);
  planeX = -dirY;
  planeY = dirX;
}

void vLine(DOME_Context ctx, int32_t x, int32_t y0, uint32_t y1, DOME_Color color) {
  y0 = fmax(0, y0);
  y1 = fmin(y1, HEIGHT);
  for (int y = y0; y <= y1; y++) {
    unsafePset(ctx, x, y, color);
  }
}

typedef struct {
  V2i mapPos;
  int side;
  V2 stepDirection;
  bool inBounds;
} CAST_RESULT;

CAST_RESULT castRay(RENDERER* renderer, V2 rayPosition, V2 rayDirection, bool ignoreDoors) {
  V2 sideDistance = {0, 0};
  CAST_RESULT result;
  /*
   sideDistance.x = sqrt(1.0 + pow((rayDirection.y / rayDirection.x), 2));
   sideDistance.y = sqrt(1.0 + pow((rayDirection.x / rayDirection.y), 2));
   */
  sideDistance.x = sqrt(1.0 + pow(rayDirection.y, 2) / pow(rayDirection.x, 2));
  sideDistance.y = sqrt(1.0 + pow(rayDirection.x, 2) / pow(rayDirection.y, 2));
  V2 nextSideDistance;
  V2 mapPos = { floor(rayPosition.x), floor(rayPosition.y) };
  V2 stepDirection = {0, 0};
  if (rayDirection.x < 0) {
    stepDirection.x = -1;
    nextSideDistance.x = (rayPosition.x - mapPos.x) * sideDistance.x;
  } else {
    stepDirection.x = 1;
    nextSideDistance.x = (mapPos.x + 1.0 - rayPosition.x) * sideDistance.x;
  }

  if (rayDirection.y < 0) {
    stepDirection.y = -1;
    nextSideDistance.y = (rayPosition.y - mapPos.y) * sideDistance.y;

  } else {
    stepDirection.y = 1;
    nextSideDistance.y = (mapPos.y + 1.0 - rayPosition.y) * sideDistance.y;
  }
  bool hit = false;
  int side = 0;
  size_t mapPitch = mapWidth; //renderer->map.width;
  TILE* tiles = renderer->map.tiles;
  while(!hit) {
    if (nextSideDistance.x < nextSideDistance.y) {
      nextSideDistance.x += sideDistance.x;
      mapPos.x += stepDirection.x;
      side = 0;
    } else {
      nextSideDistance.y += sideDistance.y;
      mapPos.y += stepDirection.y;
      side = 1;
    }
    TILE tile;
    /*
    mapPos.x = floor(mapPos.x);
    mapPos.y = floor(mapPos.y);
    */
    if (mapPos.x < 0 || mapPos.x >= renderer->map.width || mapPos.y < 0 || mapPos.y >= renderer->map.height) {
      hit = true;
      result.inBounds = false;
    } else {
      tile = tiles[(int)mapPos.y * mapPitch + (int)mapPos.x];
      result.inBounds = true;
     // Check for door and thin walls here
      if (tile.door) {
        float doorState = 1.0;
        if (tile.door) {
          doorState = ignoreDoors ? 1 : tile.state;
        }
        double adj;
        double ray_mult;
        if (side == 0) {
          adj = mapPos.x - rayPosition.x + 1.0;
          if (rayPosition.x < mapPos.x) {
            adj = adj - 1;
          }
          ray_mult = adj / rayDirection.x;
        } else {
          adj = mapPos.y - rayPosition.y + 1.0;
          if (rayPosition.y < mapPos.y) {
            adj = adj - 1;
          }
          ray_mult = adj / rayDirection.y;
        }
        float rye2 = rayPosition.y + rayDirection.y * ray_mult;
        float rxe2 = rayPosition.x + rayDirection.x * ray_mult;
        float trueDeltaX = sideDistance.x;
        float trueDeltaY = sideDistance.y;
        if (fabs(rayDirection.y) < 0.01) {
          trueDeltaY = 100;
        }
        if (fabs(rayDirection.x) < 0.01) {
          trueDeltaX = 100;
        }
        float offsetX = 0;
        float offsetY = 0;
        if (tile.door) {
          offsetX = 0.5;
          offsetY = 0.5;
        }
        // TODO- thin wall handling here
        if (side == 0) {
          float true_y_step = sqrt(trueDeltaX * trueDeltaX - 1);
          float half_step_in_y = rye2 + (stepDirection.y * true_y_step) * offsetX;
          hit = ((int)half_step_in_y == mapPos.x) && fabs(1 - 2 * (half_step_in_y - mapPos.y)) > 1 - doorState;
        } else {
          float true_x_step = sqrt(trueDeltaY * trueDeltaY - 1);
          float half_step_in_x = rxe2 + (stepDirection.x * true_x_step) * offsetY;
          hit = ((int)half_step_in_x == mapPos.x) && fabs(1 - 2 * (half_step_in_x - mapPos.x)) > 1 - doorState;
        }
      } else {
        hit = tile.solid;
      }
    }
  }

  result.mapPos = (V2i){ mapPos.x, mapPos.y };
  result.stepDirection = stepDirection;
  result.side = side;

  return result;
}

void draw(WrenVM* vm) {
  DOME_Context ctx = core->getContext(vm);
  RENDERER* renderer = wren->getSlotForeign(vm, 0);
  MAP map = renderer->map;
  double alpha = wren->getSlotDouble(vm, 1);
  // Retrieve the DOME Context from the VM. This is needed for many things.
  DOME_Color color;

  V2 rayPosition = { posX, posY };
  V2 direction = { dirX, dirY };
  V2 camera = { planeX, planeY };

  int w = WIDTH;
  int h = HEIGHT;

  int texWidth = 64;
  int texHeight = 64;

  for(int x = 0; x < w; x++) {
    // Perform DDA first
    double cameraX = 2 * (x / (double)w) - 1;
    V2 rayDirection = V2_add(direction, V2_mul(camera, cameraX));
    CAST_RESULT cast = castRay(renderer, rayPosition, rayDirection, false);
    V2 mapPos = { cast.mapPos.x, cast.mapPos.y };
    int side = cast.side;
    V2 stepDirection = cast.stepDirection;
    TILE tile = map.tiles[(int)mapPos.y * map.width + (int)mapPos.x];
    int textureId = tile.wallTextureId;

    DOME_Bitmap* texture = NULL;
    if (cast.inBounds && textureId <= sbcount(renderer->textureList)) {
      texture = renderer->textureList[textureId - 1];
      texWidth = texture->width;
      texHeight = texture->height;
    }

    double offsetX = 0;
    double offsetY = 0;
    if (tile.door) {
      offsetX = 0.5;
      offsetY = 0.5;
    }
    if (tile.door) {
      if (side == 0) {
        mapPos.x = mapPos.x + stepDirection.x * offsetX;
      } else {
        mapPos.y = mapPos.y + stepDirection.y * offsetY;
      }

    }

    double perpWallDistance;
    if (side == 0) {
      perpWallDistance = fabs((mapPos.x - rayPosition.x + (1 - stepDirection.x) / 2.0) / rayDirection.x);
    } else {
      perpWallDistance = fabs((mapPos.y - rayPosition.y + (1 - stepDirection.y) / 2.0) / rayDirection.y);
    }

    // Calculate perspective of wall-slice
    double halfH = (double)h / 2.0;
    double lineHeight = fmax(0, ((double)h / perpWallDistance));
    double drawStart = (-lineHeight / 2.0) + halfH;
    double drawEnd = (lineHeight / 2.0) + halfH;
    drawStart = clamp(0, drawStart, h - 1);
    drawEnd = clamp(0, drawEnd, h - 1);

    double wallX;
    if (side == 0) {
      wallX = rayPosition.y + perpWallDistance * rayDirection.y;
    } else {
      wallX = rayPosition.x + perpWallDistance * rayDirection.x;
    }
    wallX = wallX - floor(wallX);
    int drawWallStart = fmax(0, (int)drawStart);
    int drawWallEnd = fmin((int)ceil(drawEnd), h - 1);
    DOME_Color color;
    if (texture != NULL) {
      int texX = (int)floor(wallX * (double)(texWidth));
      if (side == 0 && rayDirection.x < 0) {
        texX = (texWidth - 1) - texX;
      }
      if (side == 1 && rayDirection.y > 0) {
        texX = (texWidth - 1) - texX;
      }

      texX = clamp(0, texX, texWidth - 1);
      assert(texX >= 0);
      assert(texX < texWidth);

      double texStep = (double)(texHeight) / lineHeight;
      double texPos = (ceil(drawStart) - halfH + (lineHeight / 2.0)) * texStep;
      for (int y = drawWallStart; y < drawWallEnd; y++) {
        int texY = ((int)texPos) % texHeight;
        assert(texY >= 0);
        assert(texY < texHeight);

        color = texture->pixels[texWidth * texY + texX]; // textureNum
        if (side == 1) {
          uint8_t alpha = color.component.a;
          color.value = (color.value >> 1) & 8355711;
          color.component.a = alpha;
        }
        assert(y < h);
        unsafePset(ctx, x, y, color);
        texPos += texStep;
      }
    } else {

      switch(textureId)
      {
        case 1:  color.value = 0xFFFF0000;  break; //red
        case 2:  color.value = 0xFF00FF00;  break; //green
        case 3:  color.value = 0xFF0000FF;   break; //blue
        case 4:  color.value = 0xFFFFFFFF;  break; //white
        default: color.value = 0xFF00FFFF; break; //yellow
      }

      //give x and y sides different brightness
      if (side == 1) {
        color.component.r /= 2;
        color.component.g /= 2;
        color.component.b /= 2;
      }
      vLine(ctx, x, drawWallStart, drawWallEnd, color);
    }
  }
  // graphics->draw(ctx, bitmap, 0, 0, DOME_DRAWMODE_BLEND);
}

DOME_EXPORT DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getAPI,
    DOME_Context ctx) {

  // Fetch the latest Core API and save it for later use.
  core = DOME_getAPI(API_DOME, DOME_API_VERSION);
  io = DOME_getAPI(API_IO, IO_API_VERSION);
  graphics = DOME_getAPI(API_GRAPHICS, GRAPHICS_API_VERSION);
  unsafePset = graphics->unsafePset;

  // DOME also provides a subset of the Wren API for accessing slots
  // in foreign methods.
  wren = DOME_getAPI(API_WREN, WREN_API_VERSION);

  core->log(ctx, "Initialising raycaster module\n");

  // Register a module with it's associated source.
  // Avoid giving the module a common name.
  core->registerModule(ctx, "raycaster", rendererModuleSource);

  core->registerClass(ctx, "raycaster", "Raycaster", allocate, finalize);
  core->registerFn(ctx, "raycaster", "Raycaster.draw(_)", draw);
  core->registerFn(ctx, "raycaster", "Raycaster.setAngle(_)", setAngle);
  core->registerFn(ctx, "raycaster", "Raycaster.loadTexture(_)", loadTexture);
  core->registerFn(ctx, "raycaster", "Raycaster.setPosition(_,_)", setPosition);

  core->registerClass(ctx, "raycaster", "WorldTile", TILE_allocate, NULL);
  core->registerFn(ctx, "raycaster", "WorldTile.solid", TILE_getSolid);
  core->registerFn(ctx, "raycaster", "WorldTile.solid=(_)", TILE_setSolid);
  core->registerFn(ctx, "raycaster", "WorldTile.door", TILE_getDoor);
  core->registerFn(ctx, "raycaster", "WorldTile.door=(_)", TILE_setDoor);
  core->registerFn(ctx, "raycaster", "WorldTile.state", TILE_getState);
  core->registerFn(ctx, "raycaster", "WorldTile.state=(_)", TILE_setState);
  core->registerFn(ctx, "raycaster", "WorldTile.mode", TILE_getMode);
  core->registerFn(ctx, "raycaster", "WorldTile.mode=(_)", TILE_setMode);
  core->registerFn(ctx, "raycaster", "WorldTile.setTextures(_)", TILE_setTextures);
  core->registerFn(ctx, "raycaster", "WorldTile.setTextures(_,_)", TILE_setTextures);
  core->registerFn(ctx, "raycaster", "WorldTile.setTextures(_,_,_)", TILE_setTextures);

  core->log(ctx, "Loading...\n");
  // bitmap = io->readImageFile(ctx, "wall.png");
  core->log(ctx, "Complete\n");

  // Returning anything other than SUCCESS here will result in the current fiber
  // aborting. Use this to indicate if your plugin initialised successfully.
  return DOME_RESULT_SUCCESS;
}

DOME_EXPORT DOME_Result PLUGIN_preUpdate(DOME_Context ctx) {
  //  DOME_Color color = graphics->pget(ctx, 0, 0);
  // core->log(ctx, "a: 0x%02hX, r: 0x%02hX, g: 0x%02hX, b: 0x%02hX\n", color.component.a, color.component.r, color.component.g, color.component.b);
  return DOME_RESULT_SUCCESS;
}

DOME_EXPORT DOME_Result PLUGIN_postUpdate(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}
DOME_EXPORT DOME_Result PLUGIN_preDraw(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}
DOME_EXPORT DOME_Result PLUGIN_postDraw(DOME_Context ctx) {
  return DOME_RESULT_SUCCESS;
}

DOME_EXPORT DOME_Result PLUGIN_onShutdown(DOME_Context ctx) {
  // io->freeBitmap(bitmap);
  return DOME_RESULT_SUCCESS;
}

