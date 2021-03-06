/*
 * decision_tree_silent.h
 *
 *  Created on: 2018/09/05
 *      Author: sysl1_1704a
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
// tree parameters
#define MAX_DEPTH 3
#define NUM_NODES (int)(pow(2, MAX_DEPTH+1) - 1)
#define MIN_SAMPLES 2

#define DEFAULT_FEATURE -1
#define DEFAULT_VALUE -1.000

/* structs */
typedef struct {
  double feature;
  double target;
} sample;

typedef struct {
  size_t id;
  size_t id_left;
  size_t id_right;
  int feature;
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
void predict(node* tree, size_t, double* predictor, double* result, size_t);

double trace_tree(node* tree, size_t, double predictor);

void print_tree(node tree[], size_t);
void print_double_array(double arr[], size_t);
void fprint_double_array(FILE* , double arr[], size_t);

void print_samples(sample samples[], size_t);

void create_training_samples(double*, double*, sample*, size_t);

void terminalize(node*, sample*, size_t, size_t);

void fit(node* tree, sample* training_samples, int len_data){
  qsort(training_samples, len_data, sizeof(sample), comp_sample);
  grow_tree(tree, 0, training_samples, 0, len_data); // the initial node is 0, and the inital index is 0.
  // NOTE: follow slice notation
}

void predict(node* tree, size_t num_nodes, double* predictor, double* result,
       size_t num_predictors) {
  size_t i;
  for (i = 0; i < num_predictors; i++) {
    result[i] = trace_tree(tree, 0, predictor[i]); // start from root node
    printf("predictor: %.3f, result: %.3f\n", predictor[i], result[i]);
  }
}

double trace_tree(node* tree, size_t node_id, double predictor) {
  //current_node = &(tree[node_id]);
  if (tree[node_id].is_terminal) {
    return tree[node_id].value;
  } else {
    // the next node; left or right?
    if (predictor < tree[node_id].value)
      return trace_tree(tree, find_left(node_id), predictor);
    else
      return trace_tree(tree, find_right(node_id), predictor);
  }
  printf("trace_tree failed.  returns -1.");
  return -1;
}

double eval_split(sample* arr, size_t slice_start, size_t slice,
		  size_t slice_end) {
  /* calculate each variance*/
  // NOTE: use slice convention
  double score_left = squared_error(arr, slice_start, slice);
  double score_right = squared_error(arr, slice, slice_end);
  // total
  return (score_left + score_right);
}

void grow_tree(node* tree, size_t node_id, sample* arr, size_t slice_start,
         size_t slice_end) {

  // node* current_node = &tree[node_id];
  // check stopping condition
  if (grow_should_stop(node_id, slice_start, slice_end)) {
    terminalize(&(tree[node_id]), arr, slice_start, slice_end);
    return;
  } else {
    /* find best split */
    // init
    size_t slice_best = slice_start;
    double score_best = (double) INT_MAX;

    // for every possible split, evaluate the score of split
    size_t slice;
    size_t init_slice = slice_start + MIN_SAMPLES; // the first slice should not be tested
    size_t last_slice = slice_end - MIN_SAMPLES + 1; // the last slice (add by 1 due to loop condition)
    for (slice = init_slice; slice < last_slice; slice++) {
      if (arr[slice - 1].feature == arr[slice].feature){
        continue; // skip a split with the same feature (corresp. removing duplicate in features)
      }
      // arr -> arr[slice_start:slice], arr[slice:slice_end]
      double score = eval_split(arr, slice_start, slice, slice_end);
      if (score < score_best) {
	// update if current score wins
	score_best = score;
	slice_best = slice;
      }
      // printf("current best slice, score: %Iu, %.3f\n", slice_best, score_best);
    }

    // set current node (leaf) values
    tree[node_id].feature = 0; // TODO: special for single feature input
    tree[node_id].value = arr[slice_best].feature;
    tree[node_id].is_terminal = false;

    // RECURSION: apply 'grow_tree' against child nodes

    // LEFT: grow_tree[left_slice_start:left_slice_end]
    grow_tree(tree, tree[node_id].id_left, arr, slice_start,
	      slice_best);
    // RIGHT: grow_tree[right_slice_start:right_slice_end]
    grow_tree(tree, tree[node_id].id_right, arr, slice_best,
	      slice_end);
 
  }
}

bool grow_should_stop(size_t node_id, size_t slice_start, size_t slice_end) {
  // stopping conditions
  bool chk_depth = (find_depth(node_id) >= MAX_DEPTH); // depth exceeds max

  int len = slice_end - slice_start;
  bool chk_len = (len <= (MIN_SAMPLES + 1)); // array size is less than or equal to min+1

  bool chk_total = chk_len | chk_depth;
  if (chk_total)
    return true;
  else
    return false;
}

void init_tree(node* tree, size_t num_nodes) {
  size_t i;
  for (i = 0; i < num_nodes; i++) {
    tree[i].id = i;
    tree[i].id_left = find_left(i);
    tree[i].id_right = find_right(i);
    tree[i].feature = DEFAULT_FEATURE;
    tree[i].value = DEFAULT_VALUE;
    tree[i].is_terminal = true;
  }
}

void create_training_samples(double* features, double* targets, sample* samples,
           size_t len) {
  size_t i;
  for (i = 0; i < len; i++) {
    samples[i].feature = features[i];
    samples[i].target = targets[i];
  }
}

void print_tree(node tree[], size_t len) {
  size_t i;
  printf("{tree: \n[");
  for (i = 0; i < len; i++) {
    printf(
     "{id: %Iu, id_left: %Iu, id_right: %Iu, feature: %d, value: %.3f, is_terminal: %d}",
     tree[i].id, tree[i].id_left, tree[i].id_right, tree[i].feature,
     tree[i].value, tree[i].is_terminal);
    // final comma should not appear
    if (i == len - 1) {
      printf("\n");
    } else {
      printf(",\n");
    }
  }
  printf("]}\n");
}

void print_double_array(double arr[], size_t len) {
  size_t i;
  for (i = 0; i < len; i++) {
    printf("%.3f, \n", (double) arr[i]);
  }
  //printf("\n");
}

void fprint_double_array(FILE* fp, double arr[], size_t len) {
  size_t i;
  for (i = 0; i < len; i++){
    fprintf(fp, "%.6f, \n", (double) arr[i]);
  }
}


void print_samples(sample samples[], size_t len) {
  size_t i;
  printf("(feature, target)\n");
  for (i = 0; i < len; i++) {
    printf("(%.3f, %.3f),\n", samples[i].feature, samples[i].target);
  }
}

// for qsort
int comparetor(const void* a, const void* b) {
  double aa = *(double*) a;
  double bb = *(double*) b;
  if (aa > bb)
    return 1;
  else if (aa < bb)
    return -1;
  else
    return 0;
}

int comp_sample(const void* a, const void* b) {
  sample aa = *(sample*) a;
  sample bb = *(sample*) b;

  if (aa.feature > bb.feature)
    return 1;
  else if (aa.feature < bb.feature)
    return -1;
  else
    return 0;
}

int find_left(int i) {
  return 2 * i + 1;
}

int find_right(int i) {
  return 2 * i + 2;
}

int find_depth(int i) {
  return (int) floor(log2(i + 1));
}

double log2(double x) {
  return log(x) / log(2);
}

double mean(double* arr, size_t slice_start, size_t slice_end) {
  double mean = 0;
  double len = slice_end - slice_start;
  size_t i = 0;
  for (i = slice_start; i <= slice_end; i++) {
    mean += arr[i];
  }
  mean = mean / len;
  return mean;
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

double var_target(sample* arr, size_t slice_start, size_t slice_end) {
  int len = slice_end - slice_start;
  if (len < 1) {
    //printf("Array length is less than 1.  Cannot calculate variance.\n");
    return 0;//arr[0].target;
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


void terminalize(node* target_node, sample* arr, size_t slice_start, size_t slice_end){
  (*target_node).is_terminal = true;
  (*target_node).value = mean_target(arr, slice_start, slice_end);
}
