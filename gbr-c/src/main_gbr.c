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
// #include <unistd.h> ... Windows does not support this

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
  sample* sample_buffer = make_samples_from_csv(path); //malloc(sizeof(sample) * n_rows);
  // memcpy(sample_buffer, training_samples, len_data);

  // print_samples(training_samples, n_rows, n_features);

  // halt();
  /* IO END */
  
  double** input_features = malloc(n_rows * sizeof(double*));
  get_features(training_samples, n_rows, input_features);
  
  // initialise slice table
  size_t*** slice_table = malloc(sizeof(size_t**) * NUM_NODES); // 3d array
  for (size_t node_id = 0; node_id < NUM_NODES; node_id++) {
    slice_table[node_id] = malloc(sizeof(size_t*) * NUM_FEATURES); // 2d array
    for (size_t i = 0; i < n_features; i++) {
      slice_table[node_id][i] = malloc(sizeof(size_t) * 2); // slice_table[i] ... i th features range.  e.g. slice_table[3] = [3, 9] means 3rd feature's range is range(3,9).
      slice_table[node_id][i][0] = 0;
      slice_table[node_id][i][1] = 1;
    }
  } 

  // initialise forest (set of trees)
  node forest[NUM_TREES][NUM_NODES];
  init_forest(forest, NUM_TREES, NUM_NODES);

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


  // init flags
  enum LR_flag* LR_flags = malloc(len_data * sizeof(enum LR_flag));
  for (size_t i = 0; i < len_data; i++) {
    LR_flags[i] = right; // init nodes are all 'right'
  }
  enum LR_flag* LR_diff = calloc(len_data, sizeof(enum LR_flag)); // init difference of LR_flag is all none.
  /*for (size_t node_id = 0; node_id < NUM_NODES; node_id++) {
    LR_flags[node_id] = calloc(len_data, sizeof(enum LR_flag));
  }*/
  //enum LR_flag* LR_best = calloc(len_data, sizeof(enum LR_flag)); // for storing best split

  // init sample ids
  size_t** ids = malloc(NUM_NODES * sizeof(size_t*));
  for (size_t node_id = 0; node_id < NUM_NODES; node_id++) {
    ids[node_id] = malloc(len_data * sizeof(size_t));
  }
  init_ids(ids, NUM_NODES, len_data);

  /* TODO: find threshold for each dimension */
  double** thresholds = malloc(sizeof(double*) * n_features);
  for (size_t i = 0; i < n_features; i++) {
	  thresholds[i] = malloc(sizeof(double) * n_rows);
  }
  
  printf("all malloc done\n");

  get_thresholds(training_samples, sample_buffer, thresholds, slice_table, n_features, len_data);

  // halt();
  
  gbr_fit(forest, NUM_TREES, NUM_NODES, residual_samples, training_samples, len_data, ids, pred, pred_by_each_tree, residual_samples_cp, input_features, n_features, slice_table, LR_flags, LR_diff, thresholds);
  
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

