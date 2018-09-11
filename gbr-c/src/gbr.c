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
#include "simple_csv.h"

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

void init_forest(node forest[][NUM_NODES], size_t, size_t);
void print_forest(node forest[][NUM_NODES], size_t, size_t);

void get_features(double*, sample*, size_t);
void get_residual(sample* , double* , sample* , size_t );
void update_prediction(double* , double* , size_t );
void update_residual(sample*, double* , sample* , size_t len_data);

void read_training_data_from_file(char* path, double* feature, double* target, size_t len);


int main(int argc, char **argv) {
  
  // dummy data
  //double input_feature[] = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 9 };
  //double input_target[] = { 30, 10, 40, 10, 50, 90, 20, 60, 50, 30, 50, 90 };

  double input_feature[LEN_DATA];
  double input_target[LEN_DATA];
  int len_data = LEN_DATA;

//  read_training_data_from_file(argv[1], input_feature, input_target, LEN_DATA);

  char* path = argv[1];
  int len = LEN_DATA;

  double data[100][2];
  read_csv(100, 2, argv[1], data);

  size_t i, j = 0;
  for (i =0; i < 100; i++){
	  input_feature[i] = data[i][0];
	  input_target[i] = data[i][1];
  }


  print_double_array(input_feature, LEN_DATA);
  print_double_array(input_target, LEN_DATA);

  fflush(stdout);
  getchar();

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
  double features[LEN_DATA];
  get_features(features, training_samples, len_data);


  // initialize prediction result
  double pred[LEN_DATA] = {0};
  double pred_by_each_tree[LEN_DATA] = {0};


  puts("Fitting...");

  sample residual_samples[LEN_DATA];
  sample residual_samples_cp[LEN_DATA];
  memcpy(residual_samples, training_samples, sizeof(sample)*len_data);

  size_t n_tree;
  for (n_tree = 0; n_tree < NUM_TREES; n_tree++){
    // fit tree
	print_samples(residual_samples, len_data);

	memcpy(residual_samples_cp, residual_samples, sizeof(sample)*len_data);
    fit(forest[n_tree], residual_samples_cp, len_data);
    // prediction
    predict(forest[n_tree], NUM_NODES, features, pred_by_each_tree, len_data); // find prediction by a tree
    print_tree(forest[n_tree], NUM_NODES);
    //scanf("%d", &a);
    fflush(stdout);
    getchar();
    update_prediction(pred, pred_by_each_tree, len_data); // pred -> pred + pred_by_each_tree
    print_double_array(pred, len_data);
    fflush(stdout);
    getchar();

    // find residual
    update_residual(residual_samples, pred, training_samples, len_data);

  }

  puts("Prediction");
  double result[LEN_DATA];
  double result_delta[LEN_DATA];

  for (n_tree = 0; n_tree < NUM_TREES; n_tree++){
    predict(forest[n_tree], NUM_NODES, input_feature, result_delta, len_data); // updates result
    update_prediction(result, result_delta, len_data); // result -> result + result_delta
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

void update_residual(sample* residual_samples, double* predictions, sample* training_samples, size_t num_samples){
	size_t i;
	for (i=0; i < num_samples; i++){
	    //training_samples[i].feature = training_samples[i].feature;
	    residual_samples[i].target = training_samples[i].target - predictions[i];
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

void read_training_data_from_file(char* path, double* feature, double* target, size_t len){

	if (access(path, F_OK) != -1) {
		FILE* stream = fopen(path, "r");
		char line[BUFFER_SIZE];
		size_t i = 0;
		while(fgets(line, BUFFER_SIZE, stream)){
			char* tmp = strdup(line);
			if (i < len) {
				feature[i] = atof(get_field(tmp, 1));
				target[i] = atof(get_field(tmp, 2));
				//printf("%.3f\n", feature[i]);
				//printf("%.3f\n", target[i]);
				++i;
			}
			free(tmp);
		}
	}
	else{
		printf("File does not exist.  Abort.");
		exit(1);
	}

}
