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
#include <unistd.h>

#include "sample.h"
#include "csv_reader.h"
#include "decision_tree.h"
#include "gbr.h"

#include "misc.h"

#include "for_debug.h"



int main(int argc, char **argv) {
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
  print_samples(training_samples, n_rows, n_features);
  /* IO END */
  
  double** input_features = malloc(n_rows * sizeof(double*));
  get_features(training_samples, n_rows, input_features);
  
  // initialise forest (set of trees)
  node forest[NUM_TREES][NUM_NODES];
  init_forest(forest, NUM_TREES, NUM_NODES);

  // sort input data
  qsort(training_samples, len_data, sizeof(sample), comp_sample);

  // initialize prediction result

  puts("Fitting...");

  //sample residual_samples[LEN_DATA] = { 0 };
  sample* residual_samples = malloc(sizeof(sample) * len_data);

  double* pred = malloc(sizeof(double) * len_data);
  double* pred_by_each_tree = malloc(sizeof(double) * len_data);
  sample* residual_samples_cp = malloc(sizeof(sample) * len_data);
  
  gbr_fit(forest, NUM_TREES, NUM_NODES, residual_samples, len_data, pred, pred_by_each_tree, residual_samples_cp, input_features);
  
  puts("Prediction");
  //double result[LEN_DATA] = {0};
  double* result = malloc(sizeof(double) * len_data);
  double* result_delta = malloc(sizeof(double) * len_data);
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
