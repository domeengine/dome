#include <stdio.h>
#include <wren.h>

struct data {
  int value1;
  int value2;
};

char* getSource(void)
{
  return "class Add {"
    "foreign add(a, b)"
    "}";
}

void printData(struct data datum) {
  printf("%i, %i\n", datum.value1, datum.value2);
}

void printFloat(float v) {
  printf("%f\n", v);
}
void printOut(char* text) {
  printf("%s", text);
}

int add(int a, int b)
{
  return a + b;
}
