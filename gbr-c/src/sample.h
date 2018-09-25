#ifndef SAMPLE_H
#define SAMPLE_H

#include <stddef.h>

typedef struct {
  double* features; //[NUM_FEATURES];
  double target;
} sample;

double mean_target(sample* arr, size_t, size_t);
double var_target(sample* arr, size_t, size_t);
double squared_error(sample* arr, size_t, size_t);

// GLOBAL VARIABLE USED BY 'comp_sapmle' //
extern size_t comp_feat_dim;
int comp_sample(const void*, const void*);

void get_features(sample* smps, size_t n_samples, double** parr);

void print_samples(sample samples[], size_t num_samples, size_t num_features);
#endif
