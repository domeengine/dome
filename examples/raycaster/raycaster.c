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
  V2 position;
  V2 direction;
  V2 cameraPlane;

  uint32_t width;
  uint32_t height;

  double* lookup;
  double* z;

  DOME_Bitmap** textureList;

  MAP map;

  OBJ* objects;
  uint64_t nextId;
} RENDERER;

typedef struct {
  size_t x;
  size_t y;
  WrenHandle* handle;
} TILE_REF;

#define worldTile(renderer, x, y) renderer->map.tiles[(int)y * renderer->map.width + (int)x]
#define getTileFrom(ref, renderer) worldTile(renderer, ref->x, ref->y)

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


void TILE_allocate(WrenVM* vm) {
  TILE_REF* ref = wren->setSlotNewForeign(vm, 0, 0, sizeof(TILE_REF));
  WrenHandle* handle = wren->getSlotHandle(vm, 1);
  ref->x = wren->getSlotDouble(vm, 2);
  ref->y = wren->getSlotDouble(vm, 3);
  ref->handle = handle;
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
  wren->ensureSlots(vm, 3); \
  wren->setSlotHandle(vm, 2, ref->handle); \
  RENDERER* renderer = wren->getSlotForeign(vm, 2); \
  TILE tile = getTileFrom(ref, renderer); \
  wren->setSlot##fieldType(vm, 0, tile.fieldName); \
}

#define TILE_SETTER(fieldName, method, fieldType) \
void TILE_set##method(WrenVM* vm) { \
  TILE_REF* ref = wren->getSlotForeign(vm, 0); \
  wren->ensureSlots(vm, 3); \
  wren->setSlotHandle(vm, 2, ref->handle); \
  RENDERER* renderer = wren->getSlotForeign(vm, 2); \
  getTileFrom(ref, renderer).fieldName = wren->getSlot##fieldType(vm, 1); \
}

TILE_GETTER(solid, Solid, Bool)
TILE_SETTER(solid, Solid, Bool)
TILE_GETTER(door, Door, Bool)
TILE_SETTER(door, Door, Bool)
TILE_GETTER(state, State, Double)
TILE_SETTER(state, State, Double)
TILE_GETTER(mode, Mode, Double)
TILE_SETTER(mode, Mode, Double)
TILE_GETTER(offset, Offset, Double)
TILE_SETTER(offset, Offset, Double)
TILE_GETTER(thin, Thin, Bool)
TILE_SETTER(thin, Thin, Bool)

void RENDERER_allocate(WrenVM* vm) {
  DOME_Context ctx = core->getContext(vm);
  RENDERER* renderer = wren->setSlotNewForeign(vm, 0, 0, sizeof(RENDERER));

  renderer->position = (V2) { 1, 1 };
  renderer->direction = (V2) { 0, 1 };
  renderer->cameraPlane = (V2) { -1, 0 };

  renderer->width = graphics->getWidth(ctx);
  renderer->height = graphics->getHeight(ctx);
  renderer->lookup = calloc(renderer->height, sizeof(double));
  for (int y = 0; y < renderer->height; y++) {
    renderer->lookup[y] = ((double)renderer->height / (2.0 * y - renderer->height));
  }
  renderer->z = calloc(renderer->width, sizeof(double));
  for (int x = 0; x < renderer->width; x++) {
    renderer->z[x] = 0;
  }
  renderer->textureList = NULL;
  renderer->objects = NULL;
  renderer->map.tiles = NULL;
  renderer->map.width = 0;
  renderer->map.height = 0;

  renderer->nextId = 0;

  // Temporary map loading
  renderer->map.width = mapHeight;
  renderer->map.height = mapWidth;
  renderer->map.tiles = calloc(mapHeight * mapWidth, sizeof(TILE));
  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      TILE tile;
      tile.solid = worldMap[y][x] != 0;
      tile.wallTextureId = worldMap[y][x];
      tile.floorTextureId = 7;
      tile.ceilingTextureId = 8;
      tile.door = worldMap[y][x] == 6;
      tile.state = tile.door ? 0.5 : 0;
      tile.locked = false;
      worldTile(renderer, x, y) = tile;
    }
  }
}

void RENDERER_pushObject(WrenVM* vm) {
  RENDERER* renderer = wren->getSlotForeign(vm, 0);
  double x = wren->getSlotDouble(vm, 1);
  double y = wren->getSlotDouble(vm, 2);
  uint32_t textureId = wren->getSlotDouble(vm, 3);

  OBJ sprite;
  sprite.id = renderer->nextId++;
  sprite.div.x = 1;
  sprite.div.y = 1;
  sprite.vMove = 0;
  sprite.textureId = textureId;
  sprite.position = (V2) {x, y};
  sb_push(renderer->objects, sprite);

  OBJ last = sb_last(renderer->objects);

  wren->setSlotDouble(vm, 0, sprite.id);
}

void RENDERER_finalize(void* data) {
  RENDERER* renderer = data;
  for (int i = 0; i < sb_count(renderer->textureList); i++) {
    io->freeBitmap(renderer->textureList[i]);
  }
  free(renderer->lookup);
  free(renderer->z);
  sb_free(renderer->textureList);
  sb_free(renderer->objects);
  free(renderer->map.tiles);
}

void RENDERER_loadTexture(WrenVM* vm) {
  DOME_Context ctx = core->getContext(vm);
  RENDERER* renderer = wren->getSlotForeign(vm, 0);
  const char* path = wren->getSlotString(vm, 1);
  DOME_Bitmap* bitmap = io->readImageFile(ctx, path);
  sb_push(renderer->textureList, bitmap);
  size_t newId = sb_count(renderer->textureList);
  wren->setSlotDouble(vm, 0, newId);
  printf("Assigning texture slot %zu\n", newId);
}

void RENDERER_setPosition(WrenVM* vm) {
  double x = wren->getSlotDouble(vm, 1);
  double y = wren->getSlotDouble(vm, 2);
  RENDERER* renderer = wren->getSlotForeign(vm, 0);
  renderer->position.x = x;
  renderer->position.y = y;
}
void RENDERER_setAngle(WrenVM* vm) {
  double angle = wren->getSlotDouble(vm, 1);
  double rads = angle * M_PI / 180.0;
  RENDERER* renderer = wren->getSlotForeign(vm, 0);
  renderer->direction.x = cos(rads);
  renderer->direction.y = sin(rads);
  renderer->cameraPlane.x = -renderer->direction.y;
  renderer->cameraPlane.y = renderer->direction.x;
}

void vLine(DOME_Context ctx, int32_t x, int32_t y0, uint32_t y1, DOME_Color color) {
  uint32_t height = graphics->getHeight(ctx);
  y0 = fmax(0, y0);
  y1 = fmin(y1, height - 1);
  for (int y = y0; y <= y1; y++) {
    unsafePset(ctx, x, y, color);
  }
}

typedef struct {
  V2 mapPos;
  int side;
  V2i stepDirection;
  bool inBounds;
} CAST_RESULT;

CAST_RESULT castRay(RENDERER* renderer, V2 rayPosition, V2 rayDirection, bool ignoreDoors) {
  V2 sideDistance = {0, 0};
  CAST_RESULT result;
  sideDistance.x = sqrt(1.0 + pow(rayDirection.y, 2) / pow(rayDirection.x, 2));
  sideDistance.y = sqrt(1.0 + pow(rayDirection.x, 2) / pow(rayDirection.y, 2));
  V2 nextSideDistance;
  V2i mapPos = { rayPosition.x, rayPosition.y };
  V2i stepDirection = {0, 0};
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
    mapPos.x = round(mapPos.x);
    mapPos.y = round(mapPos.y);
    */
    if (mapPos.x < 0 || mapPos.x >= renderer->map.width || mapPos.y < 0 || mapPos.y >= renderer->map.height) {
      hit = true;
      result.inBounds = false;
    } else {
      tile = worldTile(renderer, mapPos.x, mapPos.y);
      result.inBounds = true;
     // Check for door and thin walls here
      if (tile.thin || tile.door) {
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
        if (tile.thin) {
          offsetX = 0.5 + clamp(-0.5, tile.offset * getSign(stepDirection.x), 0.5);
          offsetY = 0.5 + clamp(-0.5, tile.offset * getSign(stepDirection.y), 0.5);
        }
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

  result.mapPos = (V2){ (int)floor(mapPos.x), (int)floor(mapPos.y) };
  result.stepDirection = stepDirection;
  result.side = side;

  return result;
}

int compareZBuffer (void* ref, const void * a, const void * b)
{
  RENDERER* renderer = ref;
  OBJ* aV = (OBJ*)a;
  OBJ* bV = (OBJ*)b;
   return V2_lengthSquared(V2_sub(renderer->position, bV->position)) - V2_lengthSquared(V2_sub(renderer->position, aV->position));
}


void RENDERER_update(WrenVM* vm) {
  DOME_Context ctx = core->getContext(vm);
  RENDERER* renderer = wren->getSlotForeign(vm, 0);

  // sortt objects by z
  // argment order is messed up because qsort_r is not standard
  qsort_r(renderer->objects, sb_count(renderer->objects), sizeof(OBJ), renderer, compareZBuffer);
}

void RENDERER_draw(WrenVM* vm) {
  DOME_Context ctx = core->getContext(vm);
  RENDERER* renderer = wren->getSlotForeign(vm, 0);
  MAP map = renderer->map;
  double alpha = wren->getSlotDouble(vm, 1);
  // Retrieve the DOME Context from the VM. This is needed for many things.
  DOME_Color color;

  V2 rayPosition = renderer->position;
  V2 direction = renderer->direction;
  V2 camera = renderer->cameraPlane;

  int w = renderer->width;
  int h = renderer->height;

  int texWidth = 64;
  int texHeight = 64;

  for(int x = 0; x < w; x++) {
    // Perform DDA first
    double cameraX = 2 * (x / (double)w) - 1;
    V2 rayDirection = V2_add(direction, V2_mul(camera, cameraX));
    CAST_RESULT cast = castRay(renderer, rayPosition, rayDirection, false);
    V2 mapPos = { cast.mapPos.x, cast.mapPos.y };
    int side = cast.side;
    V2i stepDirection = cast.stepDirection;
    TILE tile;
    int textureId = tile.wallTextureId;

    DOME_Bitmap* texture = NULL;
    if (cast.inBounds && textureId <= sb_count(renderer->textureList)) {
      tile = worldTile(renderer, mapPos.x, mapPos.y);
      textureId = tile.wallTextureId;
      if (textureId <= 0) {
      } else {
      // assert(textureId > 0);
      //printf("%i\n", textureId);
        texture = renderer->textureList[textureId - 1];
        texWidth = texture->width;
        texHeight = texture->height;
      }
    } else {
      tile = (TILE){};
      textureId = 0;
    }

    double offsetX = 0;
    double offsetY = 0;
    if (tile.door) {
      offsetX = 0.5;
      offsetY = 0.5;
    }
    if (tile.thin) {
      offsetX = 0.5 + clamp(-0.5, tile.offset * getSign(stepDirection.x), 0.5);
      offsetY = 0.5 + clamp(-0.5, tile.offset * getSign(stepDirection.y), 0.5);
    }
    if (tile.door || offsetX != 0 || offsetY != 0) {
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
    renderer->z[x] = perpWallDistance;

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
      for (int y = drawWallStart; y <= drawWallEnd; y++) {
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
        assert(y >= 0);

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

    double floorXWall;
    double floorYWall;
    if (side == 0 && rayDirection.x > 0) {
      floorXWall = mapPos.x;
      floorYWall = mapPos.y + wallX;
    } else if (side == 0 && rayDirection.x < 0) {
      floorXWall = mapPos.x + 1.0;
      floorYWall = mapPos.y + wallX;
    } else if (side == 1 && rayDirection.y > 0) {
      floorXWall = mapPos.x + wallX;
      floorYWall = mapPos.y;
    } else {
      floorXWall = mapPos.x + wallX;
      floorYWall = mapPos.y + 1.0;
    }
    double distWall = perpWallDistance;
    drawEnd = drawWallEnd;
    if (drawEnd < 0) {
      drawEnd = h - 1;
    }
    for (int y = floor(drawEnd); y < h; y++) {
      double currentDist = h / (2.0 * y - h); // TODO: make lookup on this work again
      double weight = currentDist / distWall;
      double currentFloorX = weight * floorXWall + (1.0 - weight) * rayPosition.x;
      double currentFloorY = weight * floorYWall + (1.0 - weight) * rayPosition.y;
      int floorPosX = (int)currentFloorX;
      int floorPosY = (int)currentFloorY;
      if (floorPosX < 0 || floorPosX >= map.width || floorPosY < 0 || floorPosY >= map.height) {
        continue;
      }

      assert(floorPosX >= 0);
      assert(floorPosX < mapWidth);
      assert(floorPosY >= 0);
      assert(floorPosY < mapHeight);
      TILE tile = worldTile(renderer, floorPosX, floorPosY);
      textureId = tile.floorTextureId;

      if (textureId > 0) {
        texture = renderer->textureList[textureId - 1];
        texWidth = texture->width;
        texHeight = texture->height;
        int texX = (int)(currentFloorX * texWidth) % texWidth;
        int texY = (int)(currentFloorY * texHeight) % texHeight;
        color = texture->pixels[texWidth * texY + texX]; // textureNum
        assert(y < h);
        assert(y >= 0);
        unsafePset(ctx, x, y, color);
      }
      textureId = tile.ceilingTextureId;

      if (textureId > 0) {
        texture = renderer->textureList[textureId - 1];
        texWidth = texture->width;
        texHeight = texture->height;
        int texX = (int)(currentFloorX * texWidth) % texWidth;
        int texY = (int)(currentFloorY * texHeight) % texHeight;
        color = texture->pixels[texWidth * texY + texX]; // textureNum
        assert((h - y - 1) < h);
        assert((h - y - 1) >= 0);
        unsafePset(ctx, x, h - y - 1, color);
      }
    }
  }
  size_t objCount = sb_count(renderer->objects);
  for (size_t i = 0; i < objCount; i++) {
    OBJ obj = renderer->objects[i];
    double uDiv = obj.div.x;
    double vDiv = obj.div.y;
    double vMove = obj.vMove;

    // getSpriteTransform
    V2 position = renderer->position;
    V2 dir = direction;
    V2 cam = camera;
    double invDet = 1.0 / (-camera.x * dir.y + dir.x * camera.y);
    V2 sprite = V2_sub(obj.position, position);

    V2 transform;
    transform.x = invDet * (dir.x * sprite.y - dir.y * sprite.x);
    transform.y = invDet * (cam.y * sprite.x - cam.x * sprite.y);
    // end getSpriteTransform
    if (transform.y > 0) {
      int vMoveScreen = floor(vMove / transform.y);
      int spriteScreenX = floor((w / 2.0) * (1.0 + transform.x / transform.y));
      //prevent fish eye
      int spriteHeight = floor(fabs(h / transform.y) / vDiv);
      int drawStartY = floor(((h - spriteHeight) / 2.0) + vMoveScreen);
      if (drawStartY < 0) {
        drawStartY = 0;
      }
      int drawEndY = floor(((h + spriteHeight) / 2.0) + vMoveScreen);
      if (drawEndY >= h) {
        drawEndY = h - 1;
      }
      int spriteWidth = floor((fabs(h / transform.y) / uDiv) / 2.0);
      int drawStartX = floor(spriteScreenX - spriteWidth);
      if (drawStartX < 0) {
        drawStartX = 0;
      }
      int drawEndX = floor(spriteScreenX + spriteWidth);
      if (drawEndX >= w) {
        drawEndX = w - 1;
      }

      DOME_Bitmap* texture = renderer->textureList[obj.textureId - 1];
      texWidth = texture->width;
      texHeight = texture->height;

      for (int stripe = drawStartX; stripe < drawEndX; stripe++) {
        int texX = abs((stripe - (-spriteWidth + spriteScreenX)) * texWidth / (spriteWidth * 2));

        // Conditions for this if:
        //1) it's in front of camera plane so you don't see things behind you
        //2) it's on the screen (left)
        //3) it's on the screen (right)
        //4) ZBuffer, with perpendicular distance

        if (stripe > 0 && stripe < w && transform.y < renderer->z[stripe]) {
          for (int y = drawStartY; y < drawEndY; y++) {
            int texY = fabs(((y - vMoveScreen) - (-spriteHeight / 2.0 + h / 2.0)) * texHeight / spriteHeight);
            color = texture->pixels[texWidth * texY + texX]; // textureNum
            unsafePset(ctx, stripe, y, color);
          }
        }
      }
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

  core->registerClass(ctx, "raycaster", "Raycaster", RENDERER_allocate, RENDERER_finalize);
  core->registerFn(ctx, "raycaster", "Raycaster.draw(_)", RENDERER_draw);
  core->registerFn(ctx, "raycaster", "Raycaster.update()", RENDERER_update);
  core->registerFn(ctx, "raycaster", "Raycaster.setAngle(_)", RENDERER_setAngle);
  core->registerFn(ctx, "raycaster", "Raycaster.loadTexture(_)", RENDERER_loadTexture);
  core->registerFn(ctx, "raycaster", "Raycaster.setPosition(_,_)", RENDERER_setPosition);
  core->registerFn(ctx, "raycaster", "Raycaster.f_pushObject(_,_,_)", RENDERER_pushObject);

  core->registerClass(ctx, "raycaster", "WorldTile", TILE_allocate, NULL);
  core->registerFn(ctx, "raycaster", "WorldTile.solid", TILE_getSolid);
  core->registerFn(ctx, "raycaster", "WorldTile.solid=(_)", TILE_setSolid);
  core->registerFn(ctx, "raycaster", "WorldTile.door", TILE_getDoor);
  core->registerFn(ctx, "raycaster", "WorldTile.door=(_)", TILE_setDoor);
  core->registerFn(ctx, "raycaster", "WorldTile.state", TILE_getState);
  core->registerFn(ctx, "raycaster", "WorldTile.state=(_)", TILE_setState);
  core->registerFn(ctx, "raycaster", "WorldTile.mode", TILE_getMode);
  core->registerFn(ctx, "raycaster", "WorldTile.mode=(_)", TILE_setMode);
  core->registerFn(ctx, "raycaster", "WorldTile.thin", TILE_getThin);
  core->registerFn(ctx, "raycaster", "WorldTile.thin=(_)", TILE_setThin);
  core->registerFn(ctx, "raycaster", "WorldTile.offset", TILE_getOffset);
  core->registerFn(ctx, "raycaster", "WorldTile.offset=(_)", TILE_setOffset);
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

