#include <stdio.h>
#include <math.h>

#include "misc.h"

double log2(double x) {
  return log(x) / log(2);
}

void print_double_array(double arr[], size_t len) {
  size_t i;
  for (i = 0; i < len; i++) {
    printf("%.6f, ", (double) arr[i]);
  }
  printf("\n");
}

