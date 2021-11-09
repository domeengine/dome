// typedef double[4][4] MAT;

typedef struct {
  double x;
  double y;
} V2;

V2 V2_add(V2 a, V2 b) {
  V2 result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  return result;
}

V2 V2_sub(V2 a, V2 b) {
  V2 result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  return result;
}

V2 V2_hadamard(V2 a, V2 b) {
  V2 result;
  result.x = a.x * b.x;
  result.y = a.y * b.y;
  return result;
}

V2 V2_mul(V2 a, double v) {
  V2 result;
  result.x = a.x * v;
  result.y = a.y * v;
  return result;
}


