internal void
MATH_pair(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, NUM, "x");
  ASSERT_SLOT_TYPE(vm, 2, NUM, "y");

  int64_t x = floor(wrenGetSlotDouble(vm, 1));
  int64_t y = floor(wrenGetSlotDouble(vm, 2));

  int64_t xx = (x >= 0) ? (x * 2) : ((x * -2) - 1);
  int64_t yy = (y >= 0) ? (y * 2) : ((y * -2) - 1);

  int64_t result = (xx >= yy) ? (xx * xx + xx + yy) : (yy * yy + xx);
  wrenSetSlotDouble(vm, 0, result);
}

internal void
MATH_unpair(WrenVM* vm) {
  ASSERT_SLOT_TYPE(vm, 1, NUM, "number");
  wrenEnsureSlots(vm, 2);

  double z = wrenGetSlotDouble(vm, 1);
  int64_t sqrtz = floor(sqrt(z));
  int64_t sqz = sqrtz * sqrtz;

  int64_t result[2] = { 0, 0 };

  if ((z - sqz) >= sqrtz) {
    result[0] = sqrtz;
    result[1] = z - sqz - sqrtz;
  } else {
    result[0] = z - sqz;
    result[1] = sqrtz;
  }

  result[0] = (result[0] % 2) == 0 ? (result[0] / 2) : ((result[0] + 1) / -2);
  result[1] = (result[1] % 2) == 0 ? (result[1] / 2) : ((result[1] + 1) / -2);

  wrenSetSlotNewList(vm, 0);
  wrenSetSlotDouble(vm, 1, result[0]);
  wrenInsertInList(vm, 0, 0, 1);
  wrenSetSlotDouble(vm, 1, result[1]);
  wrenInsertInList(vm, 0, 1, 1);
}
