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

#include "decision_tree_silent.h"
//#include "simple_csv.h"

#include <unistd.h>
// tree parameters

/* prototypes */

// comparetor
// tree related
//double eval_split(sample* arr, size_t, size_t, size_t);
#define LEN_DATA 100
#define BUFFER_SIZE 1024

#define N_ESTIMATORS 3
#define NUM_TREES N_ESTIMATORS

#define NUM_FEATURES 1
#define NUM_TARGETS 1

void init_forest(node forest[][NUM_NODES], size_t, size_t);
void print_forest(node forest[][NUM_NODES], size_t, size_t);

void get_features(double*, sample*, size_t);
void get_residual(sample*, double*, sample*, size_t);
void update_prediction(double*, double*, size_t);
void update_residual(sample*, double*, sample*, size_t len_data);

void read_csv(char* path, double data[][LEN_DATA], size_t len,
    size_t num_features, size_t num_targets);

// for debug
void halt();

int main(int argc, char **argv) {
  double* input_feature; //[LEN_DATA];
  double* input_target; //[LEN_DATA];
  int len_data = LEN_DATA;

  char* path = argv[1];
  int len = LEN_DATA;

  double data[NUM_FEATURES + NUM_TARGETS][LEN_DATA];
  read_csv(path, data, len, NUM_FEATURES, NUM_TARGETS);

  input_feature = data[0];
  input_target = data[1];

  sample training_samples[LEN_DATA];
  create_training_samples(input_feature, input_target, training_samples,
  LEN_DATA);

  // initialise nodes
  node forest[NUM_TREES][NUM_NODES ];

  init_forest(forest, NUM_TREES, NUM_NODES);

  // sort input data
  qsort(training_samples, LEN_DATA, sizeof(sample), comp_sample);

  double features[LEN_DATA];
  get_features(features, training_samples, len_data);

  // initialize prediction result
  double pred[LEN_DATA] = { 0 };
  double pred_by_each_tree[LEN_DATA] = { 0 };

  puts("Fitting...");

  sample residual_samples[LEN_DATA] = { 0 };
  sample residual_samples_cp[LEN_DATA] = { 0 };
  memcpy(residual_samples, training_samples, sizeof(sample) * len_data);

  size_t n_tree;
  for (n_tree = 0; n_tree < NUM_TREES; n_tree++) {
    // fit tree
    //print_samples(residual_samples, len_data);

    memcpy(residual_samples_cp, residual_samples, sizeof(sample) * len_data);

    //print_samples(residual_samples_cp, len_data);
    //halt();

    fit(forest[n_tree], residual_samples_cp, len_data);

    print_tree(forest[n_tree], NUM_NODES);
    fflush(stdout);
    // prediction
    printf("\n%Iu th tree\n", n_tree);
    predict(forest[n_tree], NUM_NODES, features, pred_by_each_tree, len_data); // find prediction by a tree
    fflush(stdout);
    //print_tree(forest[n_tree], NUM_NODES);
    //scanf("%d", &a);

    //puts("recent prediction");
    //print_double_array(pred_by_each_tree, len_data);

    update_prediction(pred, pred_by_each_tree, len_data); // pred -> pred + pred_by_each_tree

    //puts("updated prediction");
    //print_double_array(pred, len_data);


    // find residual
    update_residual(residual_samples, pred, training_samples, len_data);

  }

  //puts("Prediction");
  double result[LEN_DATA] = {0};
  double result_delta[LEN_DATA] = {0};

  for (n_tree = 0; n_tree < NUM_TREES; n_tree++) {
    predict(forest[n_tree], NUM_NODES, input_feature, result_delta, len_data); // updates result
    update_prediction(result, result_delta, len_data); // result -> result + result_delta
  }

  puts("final prediction:");

  FILE* fp;
  char* outfile = argv[2];
  fp = fopen(outfile, "w");
  fprint_double_array(fp, result, len_data);
  fclose(fp);

  return 0;
}

void get_residual(sample* residual, double* predictions,
    sample* training_samples, size_t num_samples) {
  size_t i;
  for (i = 0; i < num_samples; i++) {
    residual[i].feature = training_samples[i].feature;
    residual[i].target = training_samples[i].target - predictions[i];
  }
}

void get_features(double* features, sample* samples, size_t len_data) {
  size_t i;
  for (i = 0; i < len_data; i++) {
    features[i] = samples[i].feature;
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

void init_forest(node forest[][NUM_NODES ], size_t num_trees, size_t num_nodes) {
  size_t i;
  for (i = 0; i < num_trees; i++) {
    init_tree(forest[i], num_nodes);
  }
}

void print_forest(node forest[][NUM_NODES ], size_t num_trees, size_t num_nodes) {
  size_t i;
  for (i = 0; i < num_trees; i++) {
    printf("tree number %Iu\n", i);
    print_tree(forest[i], num_nodes);
  }
}

void read_csv(char* path, double data[][LEN_DATA], size_t len,
    size_t num_features, size_t num_targets) {
  if (access(path, F_OK) != -1) {

    FILE* stream = fopen(path, "r");
    char line[BUFFER_SIZE];
    size_t i = 0;
    size_t j = 0;
    while ((fgets(line, 1024, stream)) && (j < len)) {
      char* tmp = strdup(line);
      char* token = strtok(tmp, ",");

      while (token != NULL) {
        /* printf("i,j = %Iu, %Iu ", (i%(num_features+num_targets)), j); */
        /* printf("value= %.3f\n", atof(token)); */
        data[(i % (num_features + num_targets))][j] = atof(token);
        token = strtok(NULL, ",");
        i++;
      }
      free(tmp);
      j++;
    }
  } else {
    printf("File not found.  Abort.\n");
    exit(EXIT_FAILURE);
  }
}

void halt() {
  fflush(stdout);
  getchar();
}
