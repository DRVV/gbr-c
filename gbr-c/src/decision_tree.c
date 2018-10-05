/*
 * decision_tree.c
 *
 *  Created on: 2018/09/05
 *      Author: sysl1_1704a
 * 
 *  multi-dimensional feature ver.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>

#include "sample.h"
#include "decision_tree.h"
#include "misc.h"

#include "for_debug.h"
#include <assert.h>

void fit(node* tree, sample* training_samples, size_t len_data, size_t n_features) {
  // NOTE: grow_tree for training_samples[0:len_data]

  // init slice ranges
  size_t init_slice_ranges[DIM_FEATURES][2];
  for (size_t f = 0; f < DIM_FEATURES; f++){
    init_slice_ranges[f][0] = 0;
    init_slice_ranges[f][1] = len_data;
  }
  // the initial node_id: 0
  grow_tree(tree, 0, training_samples, len_data, n_features, init_slice_ranges);
}

void predict(node* tree, size_t num_nodes, double** predictors, double* result, size_t n_predictors) {
  size_t i;
  for (i = 0; i < n_predictors; i++) {
    result[i] = trace_tree(tree, 0, predictors[i]); // start from root node
    //printf("predictor: %.3f, result: %.3f\n", predictors[i], result[i]);
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

double eval_split(sample* samples, size_t* sample_ids, enum LR_flag* LR_flags, size_t len){
  double L_squared_sum = 0;
  double L_sum = 0;
  double L_mean = 0;
  int L_count = 0;
  
  double R_squared_sum = 0;
  double R_sum = 0;
  double R_mean = 0;
  int R_count = 0;
  
  for (size_t i = 0; i < len; i++){
    if (LR_flags[i] == left){
      L_sum += samples[sample_ids[i]].target;
      L_squared_sum += pow2(samples[sample_ids[i]].target);
      L_count++;
    }else{
      R_sum += samples[sample_ids[i]].target;
      R_squared_sum += pow2(samples[sample_ids[i]].target);
      R_count++;
    }
  }
  assert(L_count + R_count == len);
    
  L_mean = L_sum / L_count;
  R_mean = R_sum / R_count;
  
  /* calculate each variance*/
  double L_score = L_squared_sum - L_count * pow2(L_mean);
  double R_score = R_squared_sum - R_count * pow2(R_mean);

  // total score
  return L_score + R_score;
}

void grow_tree(node* tree, size_t node_id, const sample* samples, size_t* sample_ids, const double** thresholds, const size_t slice_ranges[][2], size_t len_data, size_t dim_features) {
  
  // check stopping condition
  if (grow_should_stop(node_id, dim_features, slice_ranges)) {
    terminalize(&(tree[node_id]), samples, slice_ranges);
    return;
  } else {
    // copy range table
    size_t ranges[DIM_FEATURES][2] = { 0 };
    for (size_t f = 0; f < dim_features; f++){
      for (size_t r = 0; r < 2; r++){
	ranges[f][r] = slice_ranges[f][r];
      }
    }
    /* find best split */
    size_t feat_dim;
    // init
    size_t slice_best = slice_start;
    double score_best = (double) INT_MAX;
    double value_best = 0;
    size_t dim_best = 0;

    // for every possible split, evaluate the score of split
    for (feat_dim = 0; feat_dim < dim_features; feat_dim++){
      // range for the feat_dim
      size_t slice_start = ranges[feat_dim][0];
      size_t slice_end = ranges[feat_dim][1];
      
      extern size_t comp_feat_dim;
      comp_feat_dim = feat_dim;  // MODIFYING GLOBAL VARIABLE USED BY 'comp_sample'
      qsort(samples, len_data, sizeof(sample), comp_sample); // samples gets shuffled.

      size_t slice;
      size_t init_slice = slice_start + MIN_SAMPLES; // the first slice should not be tested
      size_t last_slice = slice_end - MIN_SAMPLES + 1; // the last slice
      for (slice = init_slice; slice < last_slice; slice++) {
	// find split
	gen_LR(samples, feat_dim, thresholds[feat_dim][slice], len, LR_flags);

	// NOTE: eval_split relies on the shuffled samples (no need to specify feat_dim)
        double score = eval_split(samples, ids, dim, LR_flags, len);

        // update the score is better
        if (score < score_best) {
          score_best = score;
          slice_best = slice;
	  value_best = samples[slice].features[feat_dim];
          dim_best = feat_dim;
        }
      }
    }
    // set current node (leaf) values
    tree[node_id].feature = dim_best;
    tree[node_id].value = value_best;
    tree[node_id].is_terminal = false;

    /* RECURSION: 'grow_tree' against child nodes */

    // LEFT: samples[slice_start:slice_best]
    ranges[dim_best][0] = slice_ranges[dim_best][0]; // recover original initial slice
    ranges[dim_best][1] = slice_best;
    grow_tree(tree, tree[node_id].id_left, samples, len_data, dim_features, ranges);

    // RIGHT: samples[right_slice_start:right_slice_end]
    ranges[dim_best][0] = slice_best;
    ranges[dim_best][1] = slice_ranges[dim_best][1]; // recover original final slice
    grow_tree(tree, tree[node_id].id_right, samples, len_data, dim_features, ranges);
  }
}

bool grow_should_stop(size_t node_id, size_t dim_features, size_t slice_ranges[][2]) {
  /* stopping conditions */
  bool chk_depth = (find_depth(node_id) >= MAX_DEPTH); // depth exceeds max
  bool chk_len = true; // all array length is less than min samples + 1
  for (size_t f=0; (f < dim_features) && (chk_len) ; f++){
    int len = slice_ranges[f][1] - slice_ranges[f][0]; // slice_end - slice_start
    chk_len = chk_len && (len <= (MIN_SAMPLES + 1)); // array size is less than or equal to min+1
  }

  bool chk_total = chk_len || chk_depth;
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

// node operations

int find_left(int i) {
  return 2 * i + 1;
}

int find_right(int i) {
  return 2 * i + 2;
}

int find_depth(int i) {
  return (int) floor(log2(i + 1));
}

void terminalize(node* target_node, sample* arr, size_t slice_ranges[][2]) {
  (*target_node).is_terminal = true;
  (*target_node).value = mean_target(arr, slice_r);
}

void gen_LR(sample* samples, size_t dim, double threshold, size_t len, enum LR_flag* LR){
  for (size_t n = 0; n < len; n++){
    if (samples[n].features[dim] < threshold){
      LR[n] = left;
    } else {
      LR[n] = right;
    }
  }
}
