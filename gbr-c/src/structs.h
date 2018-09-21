#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

typedef struct {
  size_t n_rows;
  size_t n_columns;
} shape;

typedef struct {
  double* features; //[NUM_FEATURES];
  double target;
} sample;

