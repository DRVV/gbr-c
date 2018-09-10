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

#include "decision_tree.h"
// tree parameters

/* prototypes */

// comparetor

// tree related

double eval_split(sample* arr, size_t, size_t, size_t);

#define LEN_DATA 12

#define hoge 10

#define N_ESTIMATORS 3
#define NUM_TREES N_ESTIMATORS

void init_forest(node forest[][NUM_NODES], size_t, size_t);
void print_forest(node forest[][NUM_NODES], size_t, size_t);

void get_features(double*, sample*, size_t);
void get_residual(sample* , double* , sample* , size_t );
void update_prediction(double* , double* , size_t );

int main() {
  
  // dummy data
  double input_feature[] = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 9 };
  double input_target[] = { 30, 10, 40, 10, 50, 90, 20, 60, 50, 30, 50, 90 };
  int len_data = 12;

  sample training_samples[LEN_DATA];
  create_training_samples(input_feature, input_target, training_samples,
			  LEN_DATA);
  // for debug //
  print_samples(training_samples, LEN_DATA);

  // initialise nodes
  node forest[NUM_TREES][NUM_NODES];

  init_forest(forest, NUM_TREES, NUM_NODES);
  
  // sort input data
  qsort(training_samples, LEN_DATA, sizeof(sample), comp_sample);

  // initialize residuals
  sample residual_samples[LEN_DATA];
  // initialize prediction result
  double pred[LEN_DATA] = {0};
  double pred_before[LEN_DATA] = {0};

  double features[LEN_DATA];
  puts("Fitting...");
  size_t n_tree;
  for (n_tree = 0; n_tree < NUM_TREES; n_tree++){
    // fit tree
    fit(forest[n_tree], training_samples, len_data);
    // prediction
    memcpy(pred_before, pred, sizeof(double)*len_data); //pred_before = pred;


    get_features(features, training_samples, len_data);
    predict(forest[n_tree], NUM_NODES, features, pred, len_data); // rewrite pred
    // find residual
    get_residual(residual_samples, pred, training_samples, len_data);
    
    // prep for next loop; update samples & prediction
    memcpy(training_samples, residual_samples, sizeof(sample)*len_data);     //training_samples = residual_samples;
    update_prediction(pred_before, pred, len_data);
  }

  puts("Prediction");
  double result[LEN_DATA];
  double result_before[LEN_DATA];



  for (n_tree = 0; n_tree < NUM_TREES; n_tree++){
    memcpy(result_before, result, sizeof(double)*LEN_DATA);  //result_before = result;

    predict(forest[n_tree], NUM_NODES, input_feature, result, len_data);
    update_prediction(result_before, result, len_data);
  }

  print_double_array(result, len_data);
  return 0;
};

void get_residual(sample* residual, double* predictions, sample* training_samples, size_t num_samples){
  size_t i;
  for (i=0; i < num_samples; i++){
    residual[i].feature = training_samples[i].feature;
    residual[i].target = training_samples[i].target - predictions[i];
  }
}

void get_features(double* features, sample* samples, size_t len_data){
	size_t i;
	for (i =0; i < len_data; i++){
		features[i] = samples[i].feature;
	}
}


void update_prediction(double* pred, double* delta, size_t len_data){
  size_t i;
  for (i = 0; i < len_data; i++) {
    pred[i] += delta[i];
  }
}


void init_forest(node forest[][NUM_NODES], size_t num_trees, size_t num_nodes){
  size_t i;
  for (i = 0; i < num_trees; i++) {
    init_tree(forest[i], num_nodes);
  }
}

void print_forest(node forest[][NUM_NODES], size_t num_trees, size_t num_nodes){
  size_t i;
  for (i = 0; i < num_trees; i++) {
    printf("tree number %Iu\n", i);
    print_tree(forest[i], num_nodes);
  }
}
