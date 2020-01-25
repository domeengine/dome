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

double VEC_len(VEC v) {
  return sqrt(pow(v.x, 2) + pow(v.y, 2));

}

VEC VEC_add(VEC v1, VEC v2) {
  VEC result = { v1.x + v2.x, v1.y + v2.y };
  return result;
}
VEC VEC_sub(VEC v1, VEC v2) {
  VEC result = { v1.x - v2.x, v1.y - v2.y };
  return result;
}
VEC VEC_scale(VEC v, double s) {
  VEC result = { v.x * s, v.y * s };
  return result;
}

VEC VEC_neg(VEC v) {
  return VEC_scale(v, -1);
}

double VEC_dot(VEC v1, VEC v2) {
  return v1.x * v2.x + v1.y * v2.y;
}

VEC VEC_perp(VEC v) {
  VEC result = { -v.y , v.x };
  return result;
}




double max(double n1, double n2) {
  if (n1 > n2) {
    return n1;
  }
  return n2;
}

double min(double n1, double n2) {
  if (n1 < n2) {
    return n1;
  }
  return n2;
}

double mid(double n1, double n2, double n3) {
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
