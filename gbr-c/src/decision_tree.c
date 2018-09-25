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

void fit(node* tree, sample* training_samples, size_t len_data, size_t n_features) {
  // NOTE: grow_tree for training_samples[0:len_data]
  grow_tree(tree, 0, training_samples, len_data, n_features, 0, len_data); // the initial node is 0, and the inital index is 0.
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

double eval_split(sample* arr, size_t slice_start, size_t slice, size_t slice_end) {
  /* calculate each variance*/
  double score_left = squared_error(arr, slice_start, slice); // squared_error for arr[slice_start:slice]
  double score_right = squared_error(arr, slice, slice_end); // squared_error for arr[slice:slice_end]

  // total score
  return (score_left + score_right);
}

void grow_tree(node* tree, size_t node_id, sample* arr, size_t len_data, size_t dim_features, size_t slice_start, size_t slice_end) {
  // check stopping condition
  if (grow_should_stop(node_id, slice_start, slice_end)) {
    terminalize(&(tree[node_id]), arr, slice_start, slice_end);
    return;
  } else {
    /* find best split */
    size_t feat_dim;
    // init
    size_t slice_best = slice_start;
    double score_best = (double) INT_MAX;
    size_t dim_best = 0;

    // for every possible split, evaluate the score of split
    for (feat_dim = 0; feat_dim < dim_features; feat_dim++){
      extern size_t comp_feat_dim;
      comp_feat_dim = feat_dim;  // MODIFYING GLOBAL VARIABLE USED BY 'comp_sample'
      qsort(arr, len_data, sizeof(sample), comp_sample);

      size_t slice;
      size_t init_slice = slice_start + MIN_SAMPLES; // the first slice should not be tested
      size_t last_slice = slice_end - MIN_SAMPLES + 1; // the last slice
      for (slice = init_slice; slice < last_slice; slice++) {
	if (arr[slice - 1].features[feat_dim] == arr[slice].features[feat_dim]){
	  continue; // skip a split with the same feature (corresp. removing duplicate in features)
	}
	// split arr into arr[slice_start:slice] and arr[slice:slice_end]
	double score = eval_split(arr, slice_start, slice, slice_end);

	// update the score is better
	if (score < score_best) {
	  score_best = score;
	  slice_best = slice;
	  dim_best = feat_dim;
	}
	//printf("current best slice, score: %Iu, %.3f\n", slice_best, score_best);
      }
    }
    // set current node (leaf) values
    tree[node_id].feature = dim_best;
    tree[node_id].value = arr[slice_best].features[dim_best];
    tree[node_id].is_terminal = false;

    // RECURSION: 'grow_tree' against child nodes

    // LEFT: arr[slice_start:slice_best]
    grow_tree(tree, tree[node_id].id_left, arr, len_data, dim_features, slice_start, slice_best);

    // RIGHT: arr[right_slice_start:right_slice_end]
    grow_tree(tree, tree[node_id].id_right, arr, len_data, dim_features, slice_best, slice_end);
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

void terminalize(node* target_node, sample* arr, size_t slice_start,
		 size_t slice_end) {
  (*target_node).is_terminal = true;
  (*target_node).value = mean_target(arr, slice_start, slice_end);
}
