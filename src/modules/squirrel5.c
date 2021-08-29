const uint32_t SQ5_BIT_NOISE1 = 0xd2a80a3f;
const uint32_t SQ5_BIT_NOISE2 = 0xa884f197;
const uint32_t SQ5_BIT_NOISE3 = 0x6C736F4B;
const uint32_t SQ5_BIT_NOISE4 = 0xB79F3ABB;
const uint32_t SQ5_BIT_NOISE5 = 0x1b56c4f5;

uint32_t squirrel5Hash( uint32_t position, uint32_t seed ) {
	uint32_t mangledBits = position;
	mangledBits *= SQ5_BIT_NOISE1;
	mangledBits += seed;
	mangledBits ^= (mangledBits >> 9);
	mangledBits += SQ5_BIT_NOISE2;
	mangledBits ^= (mangledBits >> 11);
	mangledBits *= SQ5_BIT_NOISE3;
	mangledBits ^= (mangledBits >> 13);
	mangledBits += SQ5_BIT_NOISE4;
	mangledBits ^= (mangledBits >> 15);
	mangledBits *= SQ5_BIT_NOISE5;
	mangledBits ^= (mangledBits >> 17);
	return mangledBits;
}

void SQUIRREL5_noise(WrenVM* vm) {
  uint32_t position = wrenGetSlotDouble(vm, 1);
  uint32_t seed = wrenGetSlotDouble(vm, 2);
  wrenSetSlotDouble(vm, 0, squirrel5Hash(position, seed));
}

typedef struct {
  uint32_t seed;
  uint32_t state;
} Squirrel5;

void SQUIRREL5_allocate(WrenVM* vm) {
  Squirrel5* rng = wrenSetSlotNewForeign(vm, 0, 0, sizeof(Squirrel5));
  uint32_t seed = wrenGetSlotDouble(vm, 1);
  rng->seed = seed;
  rng->state = 0;
}

void SQUIRREL5_finalize(void* data) {}

void SQUIRREL5_float(WrenVM* vm) {
  Squirrel5* rng = wrenGetSlotForeign(vm, 0);
  double result = squirrel5Hash(rng->state++, rng->seed);
  /* cap comes from random.c and equals 0xFFFFFFFF */
  wrenSetSlotDouble(vm, 0, result / CAP);
}

