#ifndef DECISION_TREE_H
#define DECISION_TREE_H
/*
 * decision_tree.h
 * 
 *  multi-dimensional feature ver.
 */

#include <stdbool.h>
#include <math.h>
#include "sample.h"

// tree parameters
#define MAX_DEPTH 3
#define NUM_NODES 15 // (int)(pow(2, MAX_DEPTH+1) - 1)
#define MIN_SAMPLES 2

// training data format
/* #define NUM_FEATURES 1 */
/* #define DIM_FEATURES NUM_FEATURES  <-- defined for later use*/ 

#define DEFAULT_FEATURE -1
#define DEFAULT_VALUE -84.3

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

void fit(node* tree, sample* samples, size_t, size_t, size_t** slice_table);
void predict(node* tree, size_t, double** predictors, double* result, size_t);

void init_tree(node* tree, size_t);
void grow_tree(node* tree, size_t node_id, sample* arr, size_t len_data, size_t dim_features, size_t slice_ranges[][2]);
bool grow_should_stop(size_t, size_t, size_t);

double eval_split(sample* arr, size_t, size_t, size_t);

double trace_tree(node* tree, size_t, double* predictor);

void print_tree(node tree[], size_t);
void terminalize(node*, sample*, size_t, size_t);

#endif
