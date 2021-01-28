uint32_t BIT_NOISE1 = 0xB5297A4D;
uint32_t BIT_NOISE2 = 0x68E31DA4;
uint32_t BIT_NOISE3 = 0x1B56C4E9;
uint32_t CAP = 0xFFFFFFFF;

uint32_t squirrel3Hash(uint32_t position, uint32_t seed) {
  uint32_t mangled = position;
  mangled = mangled * BIT_NOISE1;
  mangled = mangled + seed;
  mangled = mangled ^ (mangled >> 8);
  mangled = mangled + BIT_NOISE2;
  mangled = mangled ^ (mangled << 8);
  mangled = mangled * BIT_NOISE3;
  mangled = mangled ^ (mangled >> 8);
  return mangled & CAP;
}

void RANDOM_noise(WrenVM* vm) {
  uint32_t position = wrenGetSlotDouble(vm, 1);
  uint32_t seed = wrenGetSlotDouble(vm, 2);
  wrenSetSlotDouble(vm, 0, squirrel3Hash(position, seed));
}

typedef struct {
  uint32_t seed;
  uint32_t state;
} RANDOM;

void RANDOM_allocate(WrenVM* vm) {
  RANDOM* rng = wrenSetSlotNewForeign(vm, 0, 0, sizeof(RANDOM));
  uint32_t seed = wrenGetSlotDouble(vm, 1);
  rng->seed = seed;
  rng->state = 0;
}

void RANDOM_finalize(void* data) {}

void RANDOM_float(WrenVM* vm) {
  RANDOM* rng = wrenGetSlotForeign(vm, 0);
  double result = squirrel3Hash(rng->state++, rng->seed);
  wrenSetSlotDouble(vm, 0, result / CAP);
}
