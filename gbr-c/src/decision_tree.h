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
#include <limits.h>

// tree parameters
#define MAX_DEPTH 3
#define NUM_NODES 15 // (int)(pow(2, MAX_DEPTH+1) - 1)
#define MIN_SAMPLES 2

// training data format
#define NUM_FEATURES 1
#define DIM_FEATURES NUM_FEATURES// <-- defined for later use*/ 

#define DEFAULT_FEATURE -1
#define DEFAULT_VALUE -84.3

#define INVALID_ID INT_MAX

typedef struct {
  size_t id; // node id, starting from 0 (root)
  size_t id_left; // the id of left child node
  size_t id_right; // the id of right child node
  int feature; // feature index (for multi dimensional features)
  double value; // threshold (if the node is not terminal) or prediction (if the node is terminal)
  bool is_terminal;
} node;

typedef struct {
  double sum; // sum of target
  double sqsum; // squared sum of target
  size_t len; // length of target
} lossState; // remembers info to calculate next loss

#define pow2(x) x*x

enum LR_flag {left = 1, right = 2, none = 0}; // none for 'false'

/* prototypes */
int find_left(int);
int find_right(int);
int find_depth(int);

void fit(node* tree, sample* training_samples, size_t** init_ids, size_t len_data, double** thresholds, size_t*** ranges, size_t n_features, enum LR_flag* init_flags, enum LR_flag* LR_diff);

void predict(node* tree, size_t, double** predictors, double* result, size_t);

void init_tree(node* tree, size_t);
void grow_tree(node* tree, size_t node_id, const sample* samples, size_t** sample_ids, enum LR_flag* LR_flags, enum LR_flag* LR_diff, double** thresholds, size_t*** slice_ranges, size_t len_entire_samples, size_t len_ids, size_t dim_features, lossState L, lossState R);

bool grow_should_stop(size_t, size_t);

double eval_split(const sample* samples, const size_t* sample_ids, const enum LR_flag* LR_flags, size_t len);
double eval_split_diff(const sample* samples, const size_t* sample_ids, const enum LR_flag* LR_diff, lossState* L_loss_state, lossState* R_loss_state, size_t len);
double trace_tree(node* tree, size_t, double* predictor);

void print_tree(node tree[], size_t);

void terminalize(node* target_node, const sample* samples, size_t len_samples, size_t* ids, size_t len_ids);


void init_ids(size_t** ids, size_t num_nodes, size_t len);

bool gen_LR(const sample* samples, size_t* sample_ids, size_t dim, double threshold, size_t len, enum LR_flag* LR);
bool gen_LR_diff(const sample* samples, size_t* sample_ids, size_t dim, double threshold, size_t len, enum LR_flag* LR, enum LR_flag* LR_diff);

size_t flag2id(const enum LR_flag* LR_list, enum LR_flag flag_to_convert, size_t** ids, size_t child_node_id, size_t parent_node_id, size_t len);

void get_thresholds(sample* samples, sample* smpl_buffer, double** thresholds, size_t*** ranges, size_t n_features, size_t len_samples);
// void get_threshold(sample*, double**);

double loss(lossState ls);
void init_LR(enum LR_flag* LR, size_t len_entire_samples, enum LR_flag);
#endif
