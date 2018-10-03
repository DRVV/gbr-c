#include <stdio.h>
#include <math.h>
#include <float.h>

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

void fprint_double_array(FILE* fp, double arr[], size_t len) {
  size_t i;
  for (i = 0; i < len; i++){
    fprintf(fp, "%.*f, \n", DECIMAL_DIG, (double) arr[i]);
  }
}
