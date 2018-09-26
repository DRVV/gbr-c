#include <stdio.h>
#include <math.h>

#include "sample.h"

size_t comp_feat_dim = 0;
double squared_error(sample* arr, size_t slice_start, size_t slice_end){
  int len = slice_end - slice_start;
  if (len < 1){
    printf("Array length is less than 1.  Cannot calculate squared error.\n");
    return 0;
  }

  double squared_sum = 0;
  double sum = 0;
  size_t i;
  for (i = slice_start; i < slice_end; i++){
    squared_sum += pow(arr[i].target, 2);
    sum += arr[i].target;
  }
  double mean = sum / len;

  return squared_sum - len * pow(mean, 2);
}

double var_target(sample* arr, size_t slice_start, size_t slice_end) {
  int len = slice_end - slice_start;
  if (len < 1) {
    printf("Array length is less than 1.  Cannot calculate variance.\n");
    return 0; //arr[0].target;
  }
  double squared = 0;
  double mean = 0;

  size_t i;
  for (i = slice_start; i < slice_end; i++) {
    //printf("%.3f\n", arr[i]);
    squared += pow(arr[i].target, 2);
    mean += arr[i].target;
  }
  mean = mean / len;

  double squared_mean = squared / len;
  double mean_squared = pow(mean, 2);
  // <X^2> - <X>^2
  return (squared_mean - mean_squared);
}

double mean_target(sample* arr, size_t slice_start, size_t slice_end) {
  double mean = 0;
  double len = slice_end - slice_start;
  size_t i = 0;
  for (i = slice_start; i < slice_end; i++) {
    mean += arr[i].target;
  }
  mean = mean / len;
  return mean;
}

void get_features(sample* smps, size_t n_samples, double** parr){
  size_t i;
  for (i=0; i<n_samples; i++){
    parr[i] = smps[i].features;
  }
}

int comp_sample(const void* a, const void* b) {
  /* +++++NOTE: GLOBAL VARIABLE 'comp_feat_dim' +++++ */
  sample aa = *(sample*) a;
  sample bb = *(sample*) b;

  if (aa.features[comp_feat_dim] > bb.features[comp_feat_dim])
    return 1;
  else if (aa.features[comp_feat_dim] < bb.features[comp_feat_dim])
    return -1;
  else
    return 0;
}

void print_samples(sample samples[], size_t num_samples, size_t num_features) {
  size_t i, dim;
  printf("([feature], target)\n");
  for (i = 0; i < num_samples; i++) {
    printf("%Iu: ([", i);
    for (dim =0; dim < num_features; dim++){
      printf("%.3f,", samples[i].features[dim]);
    }
    printf("], %.3f)\n", samples[i].target);
  }
}