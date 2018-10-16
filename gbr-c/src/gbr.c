/*
 * gbr.c
 *
 *  Created on: 2018/09/01
 *      Author: sysl1_1704a
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include <string.h>
// #include <unistd.h>

#include "sample.h"
#include "decision_tree.h"
#include "misc.h"

#include "gbr.h"
#include "for_debug.h"

void gbr_fit(node forest[][NUM_NODES], size_t n_trees, size_t n_nodes, sample* residual_samples, sample* training_samples, size_t len_data, size_t** ids, double* pred, double* pred_by_each_tree, sample* residual_samples_cp, double** features, size_t n_features, size_t*** slice_table, enum LR_flag* LR_flags, enum LR_flag* LR_diff, double** thresholds){
  /* double pred[LEN_DATA] = { 0 }; */
  /* double pred_by_each_tree[LEN_DATA] = { 0 }; */
  /* sample residual_samples_cp[LEN_DATA] = {0}; */
  // residual_samples = training_samples;

  /* STORE ORIGINAL INFO */

  size_t initial_slice_table[NUM_FEATURES][2];
  for (size_t f = 0; f < NUM_FEATURES; f++) {
    initial_slice_table[f][0] = slice_table[0][f][0]; // node id can be anything.  chose node_id=0.
    initial_slice_table[f][1] = slice_table[0][f][1];
  }

  memcpy(residual_samples, training_samples, sizeof(sample) * len_data);
  size_t n_tree;
  for (n_tree = 0; n_tree < n_trees; n_tree++) {
    // fit tree
    memcpy(residual_samples_cp, residual_samples, sizeof(sample) * len_data);


    // initialise range
    for (size_t node_id = 0; node_id < NUM_NODES; node_id++) {
      for (size_t i = 0; i < NUM_FEATURES; i++) {
        slice_table[node_id][i][0] = initial_slice_table[i][0]; //  i th feature initial index
        slice_table[node_id][i][1] = initial_slice_table[i][1]; // range(0, len_data)
      }
    }    

    char file_number[] = "xxx";
    char outfile[100] = "residuals_";
    itoa(n_tree, file_number, 10);
    strcat(outfile, file_number);
    strcat(outfile, ".csv");

    fprint_samples(outfile, residual_samples_cp, len_data, n_features);

    // recover full ids before boosting
    init_ids(ids, NUM_NODES, len_data);
    printf("n_tree: %Iu\n", n_tree);
    fit(forest[n_tree], residual_samples_cp, ids, len_data, thresholds, slice_table, n_features, LR_flags, LR_diff);

    print_tree(forest[n_tree], n_nodes);
    // halt();
    fflush(stdout);
    // prediction
    //printf("\n%Iu th tree\n", n_tree);
    predict(forest[n_tree], n_nodes, features, pred_by_each_tree, len_data); // find prediction by a tree

    update_prediction(pred, pred_by_each_tree, len_data); // pred -> pred + pred_by_each_tree
    update_residual(residual_samples, pred, training_samples, len_data); // set next training samples
  }
}

void gbr_predict(node forest[][NUM_NODES], size_t n_trees, size_t n_nodes, double** features, double* result, double* result_delta, size_t len_data){
  //double result_delta[LEN_DATA] = {0};
  size_t n_tree;
  for (n_tree = 0; n_tree < n_trees; n_tree++) {
    predict(forest[n_tree], n_nodes, features, result_delta, len_data); // updates result
    update_prediction(result, result_delta, len_data); // result -> result + result_delta
  }
}

void get_residual(sample* residual, double* predictions, sample* training_samples, size_t num_samples) {
  size_t i;
  for (i = 0; i < num_samples; i++) {
    residual[i].features = training_samples[i].features;
    residual[i].target = training_samples[i].target - predictions[i];
  }
}

void update_prediction(double* pred, double* delta, size_t len_data) {
  size_t i;
  for (i = 0; i < len_data; i++) {
    pred[i] += delta[i];
  }
}

void update_residual(sample* residual_samples, double* predictions,
    sample* training_samples, size_t num_samples) {
  size_t i;
  for (i = 0; i < num_samples; i++) {
    //training_samples[i].feature = training_samples[i].feature;
    residual_samples[i].target = training_samples[i].target - predictions[i];
  }
}

void init_forest(node forest[][NUM_NODES], size_t num_trees, size_t num_nodes) {
  size_t i;
  for (i = 0; i < num_trees; i++) {
    init_tree(forest[i], num_nodes);
  }
}

void print_forest(node forest[][NUM_NODES], size_t num_trees, size_t num_nodes) {
  size_t i;
  for (i = 0; i < num_trees; i++) {
    printf("tree number %Iu\n", i);
    print_tree(forest[i], num_nodes);
  }
}
