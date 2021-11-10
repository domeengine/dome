#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include "math.h"
#include "domath.c"
#include "sbuf.c"

// You'll need to include the DOME header
#include "dome.h"

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
  float state; // how open are we, clamped [0,1]
  uint8_t mode; // opening/closing

  float offset; // if -1, it's not thin

  // If these are negative, default to color
  int wallTextureId;
  int floorTextureId;
  int ceilingTextureId;
} TILE;

typedef struct {
  size_t width;
  size_t height;
  TILE* data;
} MAP;

typedef struct {
  MAP map;
  DOME_Bitmap** textureList;
} RENDERER;

uint32_t BLACK = 0xFF000000;
uint32_t R =  0xFFFF0000;
uint32_t B = 0xFF0000FF;
uint32_t texture[64] = {};

#define mapWidth 24
#define mapHeight 24

int worldMap[mapWidth][mapHeight]=
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
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
double planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane

static const char* source =  ""
"foreign class Raycaster {\n" // Source file for an external module
"construct init() {} \n"
"foreign setPosition(x, y) \n"
"foreign setAngle(angle) \n"
"foreign draw(alpha) \n"
"foreign loadTexture(path) \n"

"} \n";

void allocate(WrenVM* vm) {
  DOME_Context ctx = core->getContext(vm);
  WIDTH = graphics->getWidth(ctx);
  HEIGHT = graphics->getHeight(ctx);
  size_t CLASS_SIZE = sizeof(RENDERER); // This should be the size of your object's data
  RENDERER* obj = wren->setSlotNewForeign(vm, 0, 0, CLASS_SIZE);
  obj->textureList = NULL;
}

void finalize(void* data) {
  RENDERER* renderer = data;
  for (int i = 0; i < sbcount(renderer->textureList); i++) {
    io->freeBitmap(renderer->textureList[i]);
  }
  sbfree(renderer->textureList);
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

void draw(WrenVM* vm) {
  DOME_Context ctx = core->getContext(vm);
  RENDERER* renderer = wren->getSlotForeign(vm, 0);
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
  //   V2 rayDirection = V2_add(direction, V2_mul(camera, cameraX));
    // cast ray
    V2 rayDirection = {direction.x + camera.x * cameraX, direction.y + camera.y * cameraX};
    V2 sideDistance = {0, 0};
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
    int tile = 0;
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
      if (mapPos.x < 0 || mapPos.x >= mapWidth || mapPos.y < 0 || mapPos.y >= mapHeight) {
        tile = 1;
        hit = true;
      } else {
        tile = worldMap[(int)(mapPos.x)][(int)(mapPos.y)];
      }
      hit = (tile > 0);
      // Check for door and thin walls here
    }

    DOME_Bitmap* texture = NULL;
    if (tile <= sbcount(renderer->textureList)) {
      texture = renderer->textureList[tile - 1];
      texWidth = texture->width;
      texHeight = texture->height;
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

      switch(tile)
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
  core->registerModule(ctx, "raycaster", source);

  core->registerClass(ctx, "raycaster", "Raycaster", allocate, finalize);
  core->registerFn(ctx, "raycaster", "Raycaster.draw(_)", draw);
  core->registerFn(ctx, "raycaster", "Raycaster.setAngle(_)", setAngle);
  core->registerFn(ctx, "raycaster", "Raycaster.loadTexture(_)", loadTexture);
  core->registerFn(ctx, "raycaster", "Raycaster.setPosition(_,_)", setPosition);

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

