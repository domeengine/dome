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
  size_t count = sb_count(objects);

  for (size_t i = 1; i < count; i++) {
    OBJ item = objects[i];
    size_t previous = i - 1;
    while (previous >= 0 && RENDERER_compareZBuffer(renderer, &item, &objects[previous]) < 0) {
      objects[previous + 1] = objects[previous];
      previous -= 1;
    }
    objects[previous + 1] = item;
  }
}
