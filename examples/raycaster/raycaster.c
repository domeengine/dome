#include <stddef.h>
#include "math.h"
// You'll need to include the DOME header
#include "dome.h"

static GRAPHICS_API_v0* graphics;
static DOME_API_v0* core;
static WREN_API_v0* wren;
static void (*unsafePset)(DOME_Context, int32_t, int32_t, DOME_Color) = NULL;

static uint32_t WIDTH = 0;
static uint32_t HEIGHT = 0;

typedef struct {
  size_t width;
  size_t height;
  uint8_t* data;
} MAP;

typedef struct {
  MAP map;

} RENDERER;

#define mapWidth 24
#define mapHeight 24
#define screenWidth 640
#define screenHeight 480

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
"} \n";

void allocate(WrenVM* vm) {
  size_t CLASS_SIZE = sizeof(RENDERER); // This should be the size of your object's data
  void* obj = wren->setSlotNewForeign(vm, 0, 0, CLASS_SIZE);
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

void vLine(DOME_Context ctx, int32_t x, int32_t y0, uint32_t y1, uint32_t c) {
  DOME_Color color;
  color.value = c;
  y0 = fmax(0, y0);
  y1 = fmin(y1, HEIGHT);
  for (int y = y0; y <= y1; y++) {
    unsafePset(ctx, x, y, color);
  }
}

void draw(WrenVM* vm) {
  // Fetch the method argument
  double alpha = wren->getSlotDouble(vm, 1);

  // Retrieve the DOME Context from the VM. This is needed for many things.
  DOME_Context ctx = core->getContext(vm);
  DOME_Color color;
  /*
  color.value = 0xFF000000;

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      unsafePset(ctx, x, y, color);
    }
  }
  */

  int w = WIDTH;
  int h = HEIGHT;

  for(int x = 0; x < w; x++) {
    //calculate ray position and direction
    double cameraX = 2 * x / ((double)w) - 1; //x-coordinate in camera space
    double rayDirX = dirX + planeX * cameraX;
    double rayDirY = dirY + planeY * cameraX;
    //which box of the map we're in

    int mapX = posX;
    int mapY = posY;

    //length of ray from current position to next x or y-side
    double sideDistX;
    double sideDistY;

    //length of ray from one x or y-side to next x or y-side
    double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1 / rayDirX);
    double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1 / rayDirY);
    double perpWallDist;

    //what direction to step in x or y-direction (either +1 or -1)
    int stepX;
    int stepY;

    int hit = 0; //was there a wall hit?
    int side; //was a NS or a EW wall hit?


    //calculate step and initial sideDist
      if (rayDirX < 0)
      {
        stepX = -1;
        sideDistX = (posX - mapX) * deltaDistX;
      }
      else
      {
        stepX = 1;
        sideDistX = (mapX + 1.0 - posX) * deltaDistX;
      }
      if (rayDirY < 0)
      {
        stepY = -1;
        sideDistY = (posY - mapY) * deltaDistY;
      }
      else
      {
        stepY = 1;
        sideDistY = (mapY + 1.0 - posY) * deltaDistY;
      }

      //perform DDA
      while (hit == 0)
      {
        //jump to next map square, either in x-direction, or in y-direction
        if (sideDistX < sideDistY)
        {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        }
        else
        {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        //Check if ray has hit a wall
        if (worldMap[mapX][mapY] > 0) hit = 1;
      }

      //Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
      if(side == 0) perpWallDist = (sideDistX - deltaDistX);
      else          perpWallDist = (sideDistY - deltaDistY);

      //Calculate height of line to draw on screen
      int lineHeight = (int)(h / perpWallDist);

      //calculate lowest and highest pixel to fill in current stripe
      int drawStart = -lineHeight / 2 + h / 2;
      if(drawStart < 0)drawStart = 0;
      int drawEnd = lineHeight / 2 + h / 2;
      if(drawEnd >= h)drawEnd = h - 1;

      uint32_t color;
      //choose wall color
      switch(worldMap[mapX][mapY])
      {
        case 1:  color = 0xFFFF0000;  break; //red
        case 2:  color = 0xFF00FF00;  break; //green
        case 3:  color = 0xFF0000FF;   break; //blue
        case 4:  color = 0xFFFFFFFF;  break; //white
        default: color = 0xFF00FFFF; break; //yellow
      }

      //give x and y sides different brightness
      if (side == 1) {color = color / 2;}
      vLine(ctx, x, drawStart, drawEnd, color);
  }
}

DOME_EXPORT DOME_Result PLUGIN_onInit(DOME_getAPIFunction DOME_getAPI,
    DOME_Context ctx) {

  // Fetch the latest Core API and save it for later use.
  core = DOME_getAPI(API_DOME, DOME_API_VERSION);
  graphics = DOME_getAPI(API_GRAPHICS, GRAPHICS_API_VERSION);
  unsafePset = graphics->unsafePset;

  // DOME also provides a subset of the Wren API for accessing slots
  // in foreign methods.
  wren = DOME_getAPI(API_WREN, WREN_API_VERSION);

  core->log(ctx, "Initialising raycaster module\n");

  // Register a module with it's associated source.
  // Avoid giving the module a common name.
  core->registerModule(ctx, "raycaster", source);

  core->registerClass(ctx, "raycaster", "Raycaster", allocate, NULL);
  core->registerFn(ctx, "raycaster", "Raycaster.draw(_)", draw);
  core->registerFn(ctx, "raycaster", "Raycaster.setAngle(_)", setAngle);
  core->registerFn(ctx, "raycaster", "Raycaster.setPosition(_,_)", setPosition);

  WIDTH = graphics->getWidth(ctx);
  HEIGHT = graphics->getHeight(ctx);


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
  return DOME_RESULT_SUCCESS;
}

