#ifndef SAMPLE_H
#define SAMPLE_H

#include <stddef.h>

typedef struct {
  double* features; //[NUM_FEATURES];
  double target;
} sample;

double sum_target(const sample* samples, size_t len_samples, size_t* ids_summand, size_t len_summand);
double sqsum_target(const sample* samples, size_t len_samples, size_t* ids_summand, size_t len_summand);
double mean_target(const sample* samples, size_t len_samples, size_t* ids_summand, size_t len_summand);
double var_target(sample* arr, size_t, size_t);
double squared_error(sample* arr, size_t, size_t);

// GLOBAL VARIABLE USED BY 'comp_sapmle' //
extern size_t comp_feat_dim;
int comp_sample(const void*, const void*);
int comp_id(const void*, const void*);

void get_features(sample* smps, size_t n_samples, double** parr);

void print_samples(sample samples[], size_t num_samples, size_t num_features);
void fprint_samples(char* outfile, sample samples[], size_t num_samples, size_t num_features);

#endif
