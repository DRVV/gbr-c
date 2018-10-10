/*
 * gbr.c
 *
 *  Created on: 2018/09/01
 *      Author: sysl1_1704a
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include <string.h>
// #include <unistd.h>

#include "sample.h"
#include "csv_reader.h"
#include "decision_tree.h"
#include "gbr.h"

#include "misc.h"

#include "for_debug.h"

void init_samples(sample*, size_t len, size_t num_features);

int main(int argc, char **argv) {
  
  printf("INFO: DIM_FEATURES in 'decision_tree.h' is set to %d\n", DIM_FEATURES);
  
  /* IO (using malloc) */
  char* path = argv[1];
  FILE* fp = fopen(path, "r");
  shape shp = shape_of(fp);

  size_t n_rows = shp.n_rows;
  size_t n_cols = shp.n_columns;
  fclose(fp);

  size_t len_data = n_rows;

  int n_features = n_cols - DIM_TARGETS;
  sample* training_samples = make_samples_from_csv(path);
  // print_samples(training_samples, n_rows, n_features);

  // halt();
  /* IO END */
  
  double** input_features = malloc(n_rows * sizeof(double*));
  get_features(training_samples, n_rows, input_features);
  
  // initalise slice table
  size_t** slice_table = malloc(sizeof(size_t*) * n_features);
  for (size_t i = 0; i < n_features; i++)
  {
	  slice_table[i] = malloc(sizeof(size_t) * 2);
	  slice_table[i][0] = 0;
	  slice_table[i][1] = n_rows;
  }

  // initialise forest (set of trees)
  node forest[NUM_TREES][NUM_NODES];
  init_forest(forest, NUM_TREES, NUM_NODES);

  // print_forest(forest, NUM_TREES, NUM_NODES);
  // halt();

  // sort input data
  // qsort(training_samples, len_data, sizeof(sample), comp_sample);

  // print_samples(training_samples, n_rows, n_features);

  // initialize prediction result

  puts("Fitting...");

  //sample residual_samples[LEN_DATA] = { 0 };
  sample* residual_samples = malloc(len_data * sizeof(sample));
  sample* residual_samples_cp = malloc(len_data * sizeof(sample));

  init_samples(residual_samples, n_rows, n_features);
  init_samples(residual_samples_cp, n_rows, n_features);

//  residual_samples = {0};

  double* pred = calloc(len_data, sizeof(double));
  double* pred_by_each_tree = calloc(len_data, sizeof(double));

  enum LR_flag* LR_flags = calloc(len_data, sizeof(enum LR_flag));
  size_t* ids = malloc(len_data * sizeof(size_t));
  init_ids(ids, len_data);

  /* TODO: find threshold for each dimension */
  double** thresholds = NULL;
  get_threshold(training_samples, thresholds);
  
  printf("all malloc done\n");
  halt();
  
  gbr_fit(forest, NUM_TREES, NUM_NODES, residual_samples, training_samples, len_data, ids, pred, pred_by_each_tree, residual_samples_cp, input_features, n_features, slice_table, LR_flags, thresholds);
  
  puts("Prediction");
  //double result[LEN_DATA] = {0};
  double* result = calloc(len_data, sizeof(double));
  double* result_delta = calloc(len_data, sizeof(double));

  gbr_predict(forest, NUM_TREES, NUM_NODES, input_features, result, result_delta, len_data);
  puts("final prediction:");

  // output 
  FILE* fp_w;
  char* outfile = argv[2];
  fp_w = fopen(outfile, "w");
  fprint_double_array(fp_w, result, len_data);
  fclose(fp);

  return 0;
}

void init_samples(sample* smps, size_t len, size_t n_features){
  size_t i;
  for (i=0; i<len; i++){
    smps[i].features = calloc(n_features, sizeof(double));
    smps[i].target = 0;
  }
}

