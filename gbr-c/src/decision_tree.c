/*
 * decision_tree.c
 *
 *  Created on: 2018/09/05
 *      Author: sysl1_1704a
 * 
 *  multi dimensional feature
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include "csv_reader.h"
#include "for_debug.h"
// tree parameters
#define MAX_DEPTH 3
#define NUM_NODES (int)(pow(2, MAX_DEPTH+1) - 1)
#define MIN_SAMPLES 2

// training data format
#define NUM_FEATURES 2
#define DIM_FEATURES NUM_FEATURES

#define DEFAULT_FEATURE -1
#define DEFAULT_VALUE -1.000

// dummy data spec
#define LEN_DATA 12
#define ROW 2
#define COL LEN_DATA
#define NUM_SAMPLES LEN_DATA

// dummy data
double input_features[ROW][COL] = {
  { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 9 },
  //  { 9, 7, 9, 3, 2, 3, 8, 4, 6, 2, 6, 4}
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};
double input_target[] = { 30, 10, 40, 10, 50, 90, 20, 60, 50, 30, 50, 90 };
int len_data = 12;

/* structs */
/* typedef struct { */
/*   double features[NUM_FEATURES]; */
/*   double target; */
/* } sample; */

typedef struct {
  size_t id;
  size_t id_left;
  size_t id_right;
  int feature; // feature index (for multi dimensional features)
  double value;
  bool is_terminal;
} node;

/* prototypes */
int find_left(int);
int find_right(int);
int find_depth(int);

// utils
double log2(double);
double mean(double* arr, size_t, size_t);
double mean_target(sample* arr, size_t, size_t);
double var(double* arr, size_t, size_t);
double var_target(sample* arr, size_t, size_t);
double squared_error(sample* arr, size_t, size_t);

// comparetor
int comparetor(const void*, const void*);
int comp_sample(const void*, const void*);

// tree related
void init_tree(node* tree, size_t);
void grow_tree(node* tree, size_t, sample* data, size_t, size_t);
bool grow_should_stop(size_t, size_t, size_t);

double eval_split(sample* arr, size_t, size_t, size_t);

void fit(node* tree, sample* samples, int);
void predict(node* tree, size_t, double** predictors, double* result, size_t);
double trace_tree(node* tree, size_t, double* predictor);

void print_tree(node tree[], size_t);
void print_double_array(double arr[], size_t);

void create_training_samples(double(*)[NUM_SAMPLES], double*, sample*, size_t);

void terminalize(node*, sample*, size_t, size_t);

void get_features(sample* smps, size_t n_samples, double** parr);

// GLOBAL VARIABLE USED BY 'comp_sapmle' //
int comp_feat_dim = 0;


int main(int argc, char** argv) {
  
  char* path = argv[1];
  FILE* fp = fopen(path, "r");
  shape shp = shape_of(fp);

  size_t n_rows = shp.n_rows;
  size_t n_cols = shp.n_columns;
  fclose(fp);

  sample* training_samples = make_samples_from_csv(path);
  print_samples(training_samples, n_rows, n_cols - 1);

  double** input_features = malloc(n_rows * sizeof(double*));
  get_features(training_samples, n_rows, input_features);

  /* // initialise nodes */
  node tree[NUM_NODES];
  init_tree(tree, NUM_NODES);

  /* /\* ///// debug ///// *\/ */
  /* /\* puts("Init nodes:"); *\/ */
  /* /\* print_tree(tree, NUM_NODES); *\/ */

  fit(tree, training_samples, len_data);
  /* // sort input data */
  qsort(training_samples, LEN_DATA, sizeof(sample), comp_sample);

  /* /\* fit tree to input data *\/ */
  puts("Fitting...");
  grow_tree(tree, 0, training_samples, 0, len_data - 1); // the initial node is 0, and the inital index is 0.
  /* /\* /\\* output result tree data *\\/ *\/ */
  puts("Fit result:");
  print_tree(tree, NUM_NODES);

  puts("Prediction");
  double result[LEN_DATA];
  predict(tree, NUM_NODES, input_features, result, len_data);

  /* print_double_array(result, len_data); */
  return 0;
}

void fit(node* tree, sample* training_samples, int len_data) {
  grow_tree(tree, 0, training_samples, 0, len_data); // the initial node is 0, and the inital index is 0.
  // NOTE: follow slice notation
}

void predict(node* tree, size_t num_nodes, double** predictors, double* result,
	     size_t n_predictors) {
  size_t i;
  for (i = 0; i < n_predictors; i++) {
    result[i] = trace_tree(tree, 0, predictors[i]); // start from root node
    printf("predictor: %.3f, result: %.3f\n", predictors[i], result[i]);
  }
}

double trace_tree(node* tree, size_t node_id, double* predictor) {
  //current_node = &(tree[node_id]);
  if (tree[node_id].is_terminal) {
    return tree[node_id].value;
  } else {
    // the next node; left or right?
    size_t feature_idx = tree[node_id].feature;
    if (predictor[feature_idx] < tree[node_id].value)
      return trace_tree(tree, find_left(node_id), predictor);
    else
      return trace_tree(tree, find_right(node_id), predictor);
  }
  printf("trace_tree somehow failed.  Abort");
  exit(EXIT_FAILURE);
}

/* double eval_split(sample* arr, size_t slice_start, size_t slice, */
/* 		  size_t slice_end) { */
/*   /\* calculate each variance*\/ */
/*   // NOTE: use slice convention */
/*   double score_left = squared_error(arr, slice_start, slice); */
/*   double score_right = squared_error(arr, slice, slice_end); */

/*   // total */
/*   return (score_left + score_right); */
/* } */

/* void grow_tree(node* tree, size_t node_id, sample* arr, size_t slice_start, */
/* 	       size_t slice_end) { */

/*   // node* current_node = &tree[node_id]; */
/*   // check stopping condition */
/*   if (grow_should_stop(node_id, slice_start, slice_end)) { */
/*     terminalize(&(tree[node_id]), arr, slice_start, slice_end); */
/*     return; */
/*   } else { */
/*     /\* find best split *\/ */

/*     //// MODIFY GLOBAL VAR //// */
/*     size_t feat_dim; */
/*     // init */
/*     size_t slice_best = slice_start; */
/*     double score_best = (double) INT_MAX; */
/*     size_t dim_best = 0; */

/*     for (feat_dim = 0; feat_dim < DIM_FEATURES; feat_dim++){ */
/*       comp_feat_dim = feat_dim;  // GLOBAL VARIABLE USED BY 'comp_sample' */
/*       qsort(training_samples, len_data, sizeof(sample), comp_sample); */

/*       // for every possible split, evaluate the score of split */
/*       size_t slice; */
/*       size_t init_slice = slice_start + MIN_SAMPLES; // the first slice should not be tested */
/*       size_t last_slice = slice_end - MIN_SAMPLES + 1; // the last slice */
/*       for (slice = init_slice; slice < last_slice; slice++) { */
/* 	if (arr[slice - 1].features[feat_dim] == arr[slice].features[feat_dim]){ */
/* 	  continue; // skip a split with the same feature (corresp. removing duplicate in features) */
/* 	} */
/* 	// arr -> arr[slice_start:slice], arr[slice:slice_end] */
/* 	double score = eval_split(arr, slice_start, slice, slice_end); */

/* 	// update the score is better */
/* 	if (score < score_best) { */
/* 	  score_best = score; */
/* 	  slice_best = slice; */
/* 	  dim_best = feat_dim; */
/* 	} */
/* 	printf("current best slice, score: %Iu, %.3f\n", slice_best, score_best); */
/*       } */
/*     } */
/*     // set current node (leaf) values */
/*     tree[node_id].feature = dim_best; */
/*     tree[node_id].value = arr[slice_best].feature; */
/*     tree[node_id].is_terminal = false; */

/*     // RECURSION: apply 'grow_tree' against child nodes */

/*     // LEFT: grow_tree[slice_start:slice_best] */
/*     grow_tree(tree, tree[node_id].id_left, arr, slice_start, slice_best); */
/*     // RIGHT: grow_tree[right_slice_start:right_slice_end] */
/*     grow_tree(tree, tree[node_id].id_right, arr, slice_best, slice_end); */
/*   } */
/* } */

/* bool grow_should_stop(size_t node_id, size_t slice_start, size_t slice_end) { */
/*   // stopping conditions */
/*   bool chk_depth = (find_depth(node_id) > MAX_DEPTH); // depth exceeds max */

/*   int len = slice_end - slice_start; */
/*   bool chk_len = (len <= (MIN_SAMPLES + 1)); // array size is less than or equal to min+1 */

/*   bool chk_total = chk_len | chk_depth; */
/*   if (chk_total) */
/*     return true; */
/*   else */
/*     return false; */
/* } */

/* void init_tree(node* tree, size_t num_nodes) { */
/*   size_t i; */
/*   for (i = 0; i < num_nodes; i++) { */
/*     tree[i].id = i; */
/*     tree[i].id_left = find_left(i); */
/*     tree[i].id_right = find_right(i); */
/*     tree[i].feature = DEFAULT_FEATURE; */
/*     tree[i].value = DEFAULT_VALUE; */
/*     tree[i].is_terminal = true; */
/*   } */
/* } */

/* void create_training_samples(double (*features)[NUM_SAMPLES], double* targets, sample* samples, */
/* 			     size_t len) { */
/*   size_t i, dim; */
/*   for (i = 0; i < len; i++) { */
/*     for (dim = 0; dim < DIM_FEATURES; dim++){ */
/*       samples[i].features[dim] = features[dim][i]; */
/*       samples[i].target = targets[i]; */
/*     } */
/*   } */
/* } */

/* void print_tree(node tree[], size_t len) { */
/*   size_t i; */
/*   printf("{tree: \n["); */
/*   for (i = 0; i < len; i++) { */
/*     printf( */
/* 	   "{id: %Iu, id_left: %Iu, id_right: %Iu, feature: %d, value: %.3f, is_terminal: %d}", */
/* 	   tree[i].id, tree[i].id_left, tree[i].id_right, tree[i].feature, */
/* 	   tree[i].value, tree[i].is_terminal); */
/*     // final comma should not appear */
/*     if (i == len - 1) { */
/*       printf("\n"); */
/*     } else { */
/*       printf(",\n"); */
/*     } */
/*   } */
/*   printf("]}\n"); */
/* } */

/* void print_double_array(double arr[], size_t len) { */
/*   size_t i; */
/*   for (i = 0; i < len; i++) { */
/*     printf("%.3f, ", (double) arr[i]); */
/*   } */
/*   printf("\n"); */
/* } */

/* // for qsort */

/* int comp_sample(const void* a, const void* b) { */
/*   /\* +++++NOTE: GLOBAL VARIABLE 'comp_feat_dim' +++++ *\/ */
/*   sample aa = *(sample*) a; */
/*   sample bb = *(sample*) b; */

/*   if (aa.features[comp_feat_dim] > bb.features[comp_feat_dim]) */
/*     return 1; */
/*   else if (aa.features[comp_feat_dim] < bb.features[comp_feat_dim]) */
/*     return -1; */
/*   else */
/*     return 0; */
/* } */

/* int find_left(int i) { */
/*   return 2 * i + 1; */
/* } */

/* int find_right(int i) { */
/*   return 2 * i + 2; */
/* } */

/* int find_depth(int i) { */
/*   return (int) floor(log2(i + 1)); */
/* } */

/* double log2(double x) { */
/*   return log(x) / log(2); */
/* } */

/* double mean(double* arr, size_t slice_start, size_t slice_end) { */
/*   double mean = 0; */
/*   double len = slice_end - slice_start; */
/*   size_t i = 0; */
/*   for (i = slice_start; i <= slice_end; i++) { */
/*     mean += arr[i]; */
/*   } */
/*   mean = mean / len; */
/*   return mean; */
/* } */

/* double mean_target(sample* arr, size_t slice_start, size_t slice_end) { */
/*   double mean = 0; */
/*   double len = slice_end - slice_start; */
/*   size_t i = 0; */
/*   for (i = slice_start; i < slice_end; i++) { */
/*     mean += arr[i].target; */
/*   } */
/*   mean = mean / len; */
/*   return mean; */
/* } */

/* double var_target(sample* arr, size_t slice_start, size_t slice_end) { */
/*   int len = slice_end - slice_start; */
/*   if (len < 1) { */
/*     printf("Array length is less than 1.  Cannot calculate variance.\n"); */
/*     return 0; //arr[0].target; */
/*   } */
/*   double squared = 0; */
/*   double mean = 0; */

/*   size_t i; */
/*   for (i = slice_start; i < slice_end; i++) { */
/*     //printf("%.3f\n", arr[i]); */
/*     squared += pow(arr[i].target, 2); */
/*     mean += arr[i].target; */
/*   } */
/*   mean = mean / len; */

/*   double squared_mean = squared / len; */
/*   double mean_squared = pow(mean, 2); */
/*   // <X^2> - <X>^2 */
/*   return (squared_mean - mean_squared); */
/* } */

/* double squared_error(sample* arr, size_t slice_start, size_t slice_end){ */
/*   int len = slice_end - slice_start; */
/*   if (len < 1){ */
/*     printf("Array length is less than 1.  Cannot calculate squared error.\n"); */
/*     return 0; */
/*   } */

/*   double squared_sum = 0; */
/*   double sum = 0; */
/*   size_t i; */
/*   for (i = slice_start; i < slice_end; i++){ */
/*     squared_sum += pow(arr[i].target, 2); */
/*     sum += arr[i].target; */
/*   } */
/*   double mean = sum / len; */

/*   return squared_sum - len * pow(mean, 2); */
/* } */


/* void terminalize(node* target_node, sample* arr, size_t slice_start, */
/* 		 size_t slice_end) { */
/*   (*target_node).is_terminal = true; */
/*   (*target_node).value = mean_target(arr, slice_start, slice_end); */
/* } */

void get_features(sample* smps, size_t n_samples, double** parr){
  size_t i;
  for (i=0; i<n_samples; i++){
    parr[i] = smps[i].features;
  }
}
