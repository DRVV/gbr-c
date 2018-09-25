#ifndef CSV_READER_H
#define CSV_READER_H

#include <stdio.h>
#include "sample.h"

typedef struct {
  size_t n_rows;
  size_t n_columns;
} shape;

#define N_TARGETS 1
#define DIM_TARGETS N_TARGETS
#define BUFFER_SIZE 1024
// #define NUM_FEATURES 2

shape shape_of(FILE* fp);
void print_pparr(double** arr, size_t n_rows, size_t n_cols);

double** read_csv(char* path);
sample* make_samples_from_csv(char* path);

#endif
