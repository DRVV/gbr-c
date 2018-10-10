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
#include <string.h>

#include "sample.h"
#include "decision_tree.h"
#include "misc.h"

#include "for_debug.h"
#include <assert.h>

#define pow2(x) x*x

void fit(node* tree, sample* training_samples, size_t* init_ids, size_t len_data, double** thresholds, size_t n_features, enum LR_flag* init_flags) {
  // NOTE: grow_tree for training_samples[0:len_data]

  // init slice ranges
  size_t init_slice_ranges[DIM_FEATURES][2];
  for (size_t f = 0; f < DIM_FEATURES; f++){
    init_slice_ranges[f][0] = 0;
    init_slice_ranges[f][1] = len_data;
  }
  // the initial node_id: 0
  grow_tree(tree, 0, training_samples, init_ids, init_flags, thresholds, init_slice_ranges, len_data, len_data, n_features);
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

double eval_split(const sample* samples, const size_t* sample_ids, const enum LR_flag* LR_flags, size_t len){ // v2 does not require samples to be sorted
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

void grow_tree(node* tree, size_t node_id, const sample* samples, size_t* sample_ids, enum LR_flag* LR_flags, double** thresholds, const size_t slice_ranges[][2], size_t len_entire_samples, size_t len_ids, size_t dim_features) {
  
  // check stopping condition
  if (grow_should_stop(node_id, len_ids)) {
    terminalize(&(tree[node_id]), samples, len_entire_samples, sample_ids, len_ids);
  } else {
    /* 1. STORE ARG INFO */
    /* 2. FIND BEST SPLIT */
    size_t feat_dim;
    // init
    size_t slice_best;
    double score_best = (double) INT_MAX;
    double value_best = 0;
    size_t dim_best = 0;
    enum LR_flag* LR_best = LR_flags;
    // for every possible split, evaluate the score of split
    for (feat_dim = 0; feat_dim < dim_features; feat_dim++){ // 2.1 choose feature dimension to split
      // set range for the feat_dim
      size_t slice_start = slice_ranges[feat_dim][0];
      size_t slice_end = slice_ranges[feat_dim][1];
      
      for (size_t slice = slice_start; slice < slice_end; slice++) { // 2.2 choose a split and evaluate the score of the split
	// find split (-> modifies LR_flags)
	gen_LR(samples, feat_dim, thresholds[feat_dim][slice], len_ids, LR_flags); // 'LR_flags' is allocated by main function, modified by this function

	// NOTE: eval_split relies on the shuffled samples (no need to specify feat_dim)

        double score = eval_split(samples, sample_ids, LR_flags, len_ids);

        // update the score is better
        if (score < score_best) {
          score_best = score;
          slice_best = slice;
	  value_best = samples[slice].features[feat_dim];
          dim_best = feat_dim;
	  LR_best = LR_flags;
        }
      }
    }
    /* RECORD BEST-SPLIT-INFO (DIMENSION, THRESHOLD, TERMINAL OR NOT, ...) ON NODE */
    // set current node (leaf) values
    tree[node_id].feature = dim_best;
    tree[node_id].value = value_best;
    tree[node_id].is_terminal = false;

    /* 3. RECURSION: 'grow_tree' against child nodes */

    // 3.1 prep for next recursion
    // init next range 
    size_t ranges_next[DIM_FEATURES][2];
    memcpy(ranges_next, slice_ranges, sizeof(size_t) * DIM_FEATURES * 2);

    // LEFT: samples[slice_start:slice_best]
    ranges_next[dim_best][0] = slice_ranges[dim_best][0]; // recover original initial slice
    ranges_next[dim_best][1] = slice_best;
    size_t len_left = flag2id(LR_best, left, sample_ids, len_ids); // sample_ids -> array of left ids

    grow_tree(tree, tree[node_id].id_left, samples, sample_ids, LR_flags, thresholds, ranges_next, len_entire_samples, len_left, dim_features);

    // RIGHT: samples[right_slice_start:right_slice_end]
    ranges_next[dim_best][0] = slice_best;
    ranges_next[dim_best][1] = slice_ranges[dim_best][1]; // recover original final slice
    size_t len_right = flag2id(LR_best, right, sample_ids, len_ids); // sample_ids -> array of right ids
    
    grow_tree(tree, tree[node_id].id_right, samples, sample_ids, LR_flags, thresholds, ranges_next, len_entire_samples, len_right, dim_features);
  }
}

bool grow_should_stop(size_t node_id, size_t len_ids) {
  /* stopping conditions */
  bool chk_depth = (find_depth(node_id) >= MAX_DEPTH); // depth reaches/exceeds max
  bool chk_len = (len_ids <= MIN_SAMPLES + 1); // number of samples reaches minimum (+1, because len_ids == MIN_SAMPLES is not allowed in a leaf. e.g. when MIN_SAMPLES == 2, len_ids == 1, 2, 3 -> grow should stop -> true. 4 -> grow should not stop -> false.)

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

void terminalize(node* target_node, const sample* samples, size_t len_all_samples, size_t* ids, size_t len_ids) {
  (*target_node).is_terminal = true;
  (*target_node).value = mean_target(samples, len_all_samples, ids, len_ids);
}

void gen_LR(const sample* samples, size_t dim, double threshold, size_t len, enum LR_flag* LR){
  /// flag left or right for each sample (regardless of whether 'samples' is sorted)
  for (size_t n = 0; n < len; n++){
    if (samples[n].features[dim] < threshold){
      LR[n] = left;
    } else {
      LR[n] = right;
    }
  }
}

size_t flag2id(const enum LR_flag* LR_list, enum LR_flag flag_to_convert, size_t* id_buffer, size_t len){
  size_t count = 0;
  for (size_t i = 0; i < len; i++){
    if (LR_list[i] == flag_to_convert){
      id_buffer[i] = flag_to_convert;
      count++;
    } else {
      id_buffer[i] = none;
    }
  }
  return count;
}

void init_ids(size_t* ids, size_t len){
  for (size_t i=0; i<len; i++){
    ids[i] = i;
  }
}

void get_threshold(sample* samples, double** threshold){
  /* TODO: IMPLEMENT */
}
