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

#include "decision_tree.h"
// tree parameters

/* prototypes */

// comparetor

// tree related

double eval_split(sample* arr, size_t, size_t, size_t);

#define LEN_DATA 12

#define hoge 10

#define N_ESTIMATORS 3

int main() {
  
  // dummy data
  double input_feature[] = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 9 };
  double input_target[] = { 30, 10, 40, 10, 50, 90, 20, 60, 50, 30, 50, 90 };
  int len_data = 12;

  sample training_samples[LEN_DATA];
  create_training_samples(input_feature, input_target, training_samples,
			  LEN_DATA);
  print_samples(training_samples, LEN_DATA);
  //print_double_array(input_data, len_data);

  // initialise nodes
  node boosting_trees[N_ESTIMATORS][NUM_NODES];
  
  node tree[NUM_NODES];
  init_tree(tree, NUM_NODES);

  ///// debug /////
  puts("Init nodes:");
  print_tree(tree, NUM_NODES);

  /* // sort input data */
  qsort(training_samples, LEN_DATA, sizeof(sample), comp_sample);

  /* fit tree to input data */
  puts("Fitting...");
  grow_tree(tree, 0, training_samples, 0, len_data - 1); // the initial node is 0, and the inital index is 0.

  /* output result tree data */
  puts("Fit result:");
  print_tree(tree, NUM_NODES);

  puts("Prediction");
  double result[LEN_DATA];
  predict(tree, NUM_NODES, input_feature, result, len_data);

  print_double_array(result, len_data);
  
  //find residual
  
  get_residual()
  return 0;
};

void get_residual(sample* residual, double* predictions, sample* training_samples, size_t num_samples){
  size_t i;
  for (i=0; i < num_samples; i++){
    residual[i].feature = training_samples[i].feature;
    residual[i].target = training_samples[i].target - predictions[i];
  }
}
