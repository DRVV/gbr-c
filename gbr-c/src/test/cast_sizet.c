#include <stdio.h>

int main(){
  size_t a = 10;
  double b = 10.00;

  printf("a: %Iu\n", a);
  printf("b: %.3f\n", b);

  printf("cast b: %Iu\n", (size_t)b);
  printf("a == b ?: %d\n", a == (size_t)b);
  
}
