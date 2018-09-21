/*
 * decision_tree.h
 * 
 *  multi-dimensional feature ver.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include "csv_reader.h"
#include "for_debug.h"
//#include "structs.h" <- csv_reader.h includes this

// tree parameters
#define MAX_DEPTH 3
#define NUM_NODES (int)(pow(2, MAX_DEPTH+1) - 1)
#define MIN_SAMPLES 2

// training data format
/* #define NUM_FEATURES 1 */
/* #define DIM_FEATURES NUM_FEATURES  <-- defined for later use*/ 

#define DEFAULT_FEATURE -1
#define DEFAULT_VALUE -1.000

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
double var_target(sample* arr, size_t, size_t);
double squared_error(sample* arr, size_t, size_t);

// comparetor
int comp_sample(const void*, const void*);

// tree related
void init_tree(node* tree, size_t);
//void grow_tree(node* tree, size_t, sample* data, size_t, size_t, size_t, size_t);

void grow_tree(node* tree, size_t node_id, sample* arr, size_t len_data, size_t dim_features, size_t slice_start, size_t slice_end);

bool grow_should_stop(size_t, size_t, size_t);

double eval_split(sample* arr, size_t, size_t, size_t);

void fit(node* tree, sample* samples, size_t, size_t);
void predict(node* tree, size_t, double** predictors, double* result, size_t);
double trace_tree(node* tree, size_t, double* predictor);

void print_tree(node tree[], size_t);
void print_double_array(double arr[], size_t);

void terminalize(node*, sample*, size_t, size_t);

void get_features(sample* smps, size_t n_samples, double** parr);

// GLOBAL VARIABLE USED BY 'comp_sapmle' //
int comp_feat_dim = 0;
