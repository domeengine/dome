#include "renderer.wren.inc"
#define worldTile(renderer, x, y) renderer->map.tiles[(int)y * renderer->map.width + (int)x]
#define getTileFrom(ref, renderer) worldTile(renderer, ref->x, ref->y)

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

  // An ID of zero means the entry is not in use.
  // Valid IDs are > 0
  renderer->nextId = 1;
}


void RENDERER_pushObject(WrenVM* vm) {
  RENDERER* renderer = wren->getSlotForeign(vm, 0);
  double x = wren->getSlotDouble(vm, 1);
  double y = wren->getSlotDouble(vm, 2);
  uint32_t textureId = wren->getSlotDouble(vm, 3);

  OBJ sprite;
  // Safety init
  memset(&sprite, 0, sizeof(OBJ));
  sprite.id = renderer->nextId++;
  sprite.div.x = 1;
  sprite.div.y = 1;
  sprite.vMove = 0;
  sprite.textureId = textureId;
  sprite.position = (V2) {x, y};

  sb_push(renderer->objects, sprite);
  renderer->objectCount++;

  wren->setSlotDouble(vm, 0, sprite.id);
}


void RENDERER_finalize(void* data) {
  RENDERER* renderer = data;
  free(renderer->lookup);
  free(renderer->z);
  free(renderer->map.tiles);

  for (int i = 0; i < sb_count(renderer->textureList);  i++) {
    io->freeBitmap(renderer->textureList[i]);
  }
  sb_free(renderer->textureList);
  sb_free(renderer->objects);
}

void RENDERER_loadTexture(WrenVM* vm) {
  DOME_Context ctx = core->getContext(vm);
  RENDERER* renderer = wren->getSlotForeign(vm, 0);
  const char* path = wren->getSlotString(vm, 1);
  DOME_Bitmap* bitmap = io->readImageFile(ctx, path);
  sb_push(renderer->textureList, bitmap);
  if (renderer->textureList == NULL) {
    abort();
  }
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

void RENDERER_setDimensions(WrenVM* vm) {
  uint64_t width = wren->getSlotDouble(vm, 1);
  uint64_t height = wren->getSlotDouble(vm, 2);
  RENDERER* renderer = wren->getSlotForeign(vm, 0);
  renderer->map.width = width;
  renderer->map.height = height;
  renderer->map.tiles = realloc(renderer->map.tiles, width * height * sizeof(TILE));
  memset(renderer->map.tiles, 0, width * height * sizeof(TILE));
}

OBJ* RENDERER_getObject(RENDERER* renderer, uint64_t id) {
  OBJ* objects = renderer->objects;
  size_t count = renderer->objectCount;

  for (size_t i = 0; i < count; i++) {
    OBJ* item = objects + i;
    if (item->id == id) {
      return item;
    }
  }
  return NULL;
}

int RENDERER_compareZBuffer (void* ref, const void * a, const void * b)
{
  RENDERER* renderer = ref;
  OBJ* aV = (OBJ*)a;
  OBJ* bV = (OBJ*)b;
  return V2_lengthSquared(V2_sub(renderer->position, bV->position)) - V2_lengthSquared(V2_sub(renderer->position, aV->position));
}

void RENDERER_sort(RENDERER* renderer) {
  // Assume that the list of objects is /nearly/ sorted, frame over frame
  // So insertion sort gives the best performance on average.

  OBJ* objects = renderer->objects;
  size_t count = renderer->objectCount;

  for (size_t i = 1; i < count; i++) {
    OBJ item = objects[i];
    assert(item.id > 0);
    size_t previous = i;
    while (previous > 0 && RENDERER_compareZBuffer(renderer, &item, &objects[previous - 1]) <= 0) {
      objects[previous] = objects[previous - 1];
      previous -= 1;
    }
    objects[previous] = item;
  }
}

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
  size_t mapPitch = renderer->map.width;
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

    if (mapPos.x < 0 || mapPos.x >= renderer->map.width || mapPos.y < 0 || mapPos.y >= renderer->map.height) {
      hit = true;
      result.inBounds = false;
    } else {
      if (renderer->map.width * renderer->map.height == 0) {
        tile = VOID_TILE;
      } else {
        tile = worldTile(renderer, mapPos.x, mapPos.y);
      }
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


void RENDERER_update(WrenVM* vm) {
  DOME_Context ctx = core->getContext(vm);
  RENDERER* renderer = wren->getSlotForeign(vm, 0);

  // sort objects by z
  RENDERER_sort(renderer);
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
      if (renderer->map.width * renderer->map.height == 0) {
        tile = VOID_TILE;
      } else {
        tile = worldTile(renderer, mapPos.x, mapPos.y);
      }
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
      assert(floorPosX < renderer->map.width);
      assert(floorPosY >= 0);
      assert(floorPosY < renderer->map.height);
      TILE tile;
      if (renderer->map.width * renderer->map.height == 0) {
        tile = VOID_TILE;
      } else {
        tile = worldTile(renderer, floorPosX, floorPosY);
      }
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
  size_t objCount = renderer->objectCount;
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

void RENDERER_register(DOME_Context ctx) {
  core->registerClass(ctx, "raycaster", "Raycaster", RENDERER_allocate, RENDERER_finalize);
  core->registerFn(ctx, "raycaster", "Raycaster.draw(_)", RENDERER_draw);
  core->registerFn(ctx, "raycaster", "Raycaster.update()", RENDERER_update);
  core->registerFn(ctx, "raycaster", "Raycaster.setAngle(_)", RENDERER_setAngle);
  core->registerFn(ctx, "raycaster", "Raycaster.loadTexture(_)", RENDERER_loadTexture);
  core->registerFn(ctx, "raycaster", "Raycaster.setPosition(_,_)", RENDERER_setPosition);
  core->registerFn(ctx, "raycaster", "Raycaster.setDimensions(_,_)", RENDERER_setDimensions);
  core->registerFn(ctx, "raycaster", "Raycaster.f_pushObject(_,_,_)", RENDERER_pushObject);
}


