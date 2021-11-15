
#include <stdint.h>

double
getSign(double n) {
  if (n < 0.0001) {
    return -1;
  }
  if (n > 0.0001) {
    return 1;
  }
  return 0;
}

inline double
clamp(double min, double x, double max) {
  if (x < min) {
    return min;
  }
  if (x > max) {
    return max;
  }
  return x;
  // return fmax(min, fmin(x, max));
}


typedef struct {
  int32_t x;
  int32_t y;
} V2i;

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

double V2_length(V2 a) {
  return sqrt(pow(a.x, 2) + pow(a.y, 2));
}

double V2_lengthSquared(V2 a) {
  return pow(a.x, 2) + pow(a.y, 2);
}

typedef struct {
  double values[2 * 2];
} M2;

typedef struct {
  uint32_t values[2 * 2];
} M2i;

typedef struct {
  double values[3 * 3];
} M3;

typedef struct {
  uint32_t values[3 * 3];
} M3i;

typedef struct {
  double values[4 * 4];
} M4;

typedef struct {
  uint32_t values[4 * 4];
} M4i;
