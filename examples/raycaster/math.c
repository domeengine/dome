typedef double[4][4] MAT;

typedef struct {
  double x;
  double y;
} V2;

inline V2 V_add(V2 a, V2 b) {
  V2 result;
  result.x = a.x + b.x;
  result.y = a.y + b.y;
  return result;
}

inline V2 V_sub(V2 a, V2 b) {
  V2 result;
  result.x = a.x - b.x;
  result.y = a.y - b.y;
  return result;
}

inline V2 V_hadamard(V2 a, V2 b) {
  V2 result;
  result.x = a.x * b.x;
  result.y = a.y * b.y;
  return result;
}

inline V2 V_mul(V2 a, double v) {
  V2 result;
  result.x = a.x * v;
  result.y = a.y * v;
  return result;
}


