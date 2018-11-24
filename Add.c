#include <stdio.h>

int add(int x, int y) { return x + y; }

int main() {
  int x = 0;
  int y = 5;
  int z = add(x, y);
  printf("%d\n", z);
  return 0;
}
