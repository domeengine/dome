/*
 math.c
 */
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
