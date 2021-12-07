/*
 math.c
 */

typedef struct {
  int32_t x;
  int32_t y;
} iVEC;
typedef struct {
  double x;
  double y;
} VEC;

internal double
VEC_len(VEC v) {
  return sqrt(pow(v.x, 2) + pow(v.y, 2));

}

internal VEC
VEC_add(VEC v1, VEC v2) {
  VEC result = { v1.x + v2.x, v1.y + v2.y };
  return result;
}
internal VEC
VEC_sub(VEC v1, VEC v2) {
  VEC result = { v1.x - v2.x, v1.y - v2.y };
  return result;
}
internal VEC
VEC_scale(VEC v, double s) {
  VEC result = { v.x * s, v.y * s };
  return result;
}

internal VEC
VEC_neg(VEC v) {
  return VEC_scale(v, -1);
}

internal double
VEC_dot(VEC v1, VEC v2) {
  return v1.x * v2.x + v1.y * v2.y;
}

internal VEC
VEC_perp(VEC v) {
  VEC result = { -v.y , v.x };
  return result;
}

inline internal float
lerp(float a, float b, float f) {
  return (a * (1.0 - f)) + (b * f);
}

inline internal void
swap(float* a, float* b) {
    float temp = *a;
    *a = *b;
    *b = temp;
}

internal int64_t
max(int64_t n1, int64_t n2) {
  if (n1 > n2) {
    return n1;
  }
  return n2;
}

internal int64_t
min(int64_t n1, int64_t n2) {
  if (n1 < n2) {
    return n1;
  }
  return n2;
}

internal double
fmid(double n1, double n2, double n3) {
  double temp;
  if (n1 > n3) {
    temp = n1;
    n1 = n3;
    n3 = temp;
  }
  if (n1 > n2) {
    temp = n1;
    n1 = n2;
    n2 = temp;
  }
  if (n2 < n3) {
    return n2;
  } else {
    return n3;
  }
}

internal int64_t
mid(int64_t n1, int64_t n2, int64_t n3) {
  int64_t temp;
  if (n1 > n3) {
    temp = n1;
    n1 = n3;
    n3 = temp;
  }
  if (n1 > n2) {
    temp = n1;
    n1 = n2;
    n2 = temp;
  }
  if (n2 < n3) {
    return n2;
  } else {
    return n3;
  }
}

internal uint64_t
gcd(uint64_t a, uint64_t b) {
  uint64_t t = b;
  while (b != 0) {
    t = b;
    b = a % b;
    a = t;
  }
  return a;
}
