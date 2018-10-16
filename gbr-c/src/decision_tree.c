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

void fit(node* tree, sample* training_samples, size_t** init_ids, size_t len_data, double** thresholds, size_t*** ranges, size_t n_features, enum LR_flag* init_flags, enum LR_flag* LR_diff) {
  // NOTE: grow_tree for training_samples[0:len_data]

  // the initial node_id: 0
  size_t root_node_id = 0;
  lossState L_init_lossState = {.sum = 0,
                                .sqsum = 0,
                                .len = 0};
  lossState R_init_lossState = {.sum = sum_target(training_samples, len_data, init_ids[root_node_id], len_data),
                                .sqsum = sqsum_target(training_samples, len_data, init_ids[root_node_id], len_data),
                                .len = len_data};

  grow_tree(tree, root_node_id, training_samples, init_ids, init_flags, LR_diff, thresholds, ranges, len_data, len_data, n_features, L_init_lossState, R_init_lossState);
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
  assert(L_count && R_count); // assert both L, R counts are nonzero

  L_mean = L_sum / L_count;
  R_mean = R_sum / R_count;
  
  /* calculate each variance*/
  double L_score = L_squared_sum - L_count * pow2(L_mean);
  double R_score = R_squared_sum - R_count * pow2(R_mean);

  // total score
  return L_score + R_score;
}
double eval_split_diff(const sample* samples, const size_t* sample_ids, const enum LR_flag* LR_diff, lossState *L_loss_state, lossState *R_loss_state, size_t len_ids){ // v2 does not require samples to be sorted
  /// evaluate the difference of splitting score 
  for (size_t i = 0; i < len_ids; i++){
    size_t sample_id = sample_ids[i];
    if (LR_diff[sample_id] == left){ // diff is left === previous was right.
      // update sum
      L_loss_state->sum += samples[sample_id].target; // add to L
      R_loss_state->sum -= samples[sample_id].target; // remove from R
      // update squared sum
      L_loss_state->sqsum += pow2(samples[sample_id].target);
      R_loss_state->sqsum -= pow2(samples[sample_id].target);
      // update length
      L_loss_state->len++;
      R_loss_state->len--;
    } else if (LR_diff[sample_id] == right) {
      // update sum
      L_loss_state->sum -= samples[sample_id].target;
      R_loss_state->sum += samples[sample_id].target;
      // update squared sum
      L_loss_state->sqsum -= pow2(samples[sample_id].target);
      R_loss_state->sqsum += pow2(samples[sample_id].target);
      // update length
      L_loss_state->len--;
      R_loss_state->len++;
    } else {
      // do nothing
    }
  }
  // assert(L_loss_state->len + R_loss_state->len == len);
  // assert(L_loss_state.len && R_loss_state.len); // assert both L, R counts are nonzero

  /* calculate each variance*/
  double L_score = loss(*L_loss_state);
  double R_score = loss(*R_loss_state);

  // total score
  return L_score + R_score;
}

void grow_tree(node* tree, size_t node_id, const sample* samples, size_t** sample_ids, enum LR_flag* LR_flags, enum LR_flag* LR_diff, double** thresholds, size_t*** slice_ranges, size_t len_entire_samples, size_t len_ids, size_t dim_features, lossState L_ls, lossState R_ls) {
  lossState L_loss_state = L_ls; //{.sum = 0, .sqsum = 0, .len = 0}; // assume all right flag
  lossState R_loss_state = R_ls; //{.sum = sum_target(samples, len_entire_samples, sample_ids[node_id], len_ids),
		//	    .sqsum = sqsum_target(samples, len_entire_samples, sample_ids[node_id], len_ids),
		//	    .len = len_ids};

  lossState L_loss_state_best;
  lossState R_loss_state_best;
    
  printf("  node id: %Iu\n", node_id);
  /* 1. CHECK STOPPING CONDITION */
  if (grow_should_stop(node_id, len_ids)) {
    terminalize(&(tree[node_id]), samples, len_entire_samples, sample_ids[node_id], len_ids);
    return;
  }
  
  /* 2. FIND BEST SPLIT */
  // init
  size_t slice_best;
  double score_best; //= (double)INT_MAX;
  if (R_loss_state.len) // Either L or R length is 0.
    score_best = loss(R_loss_state); // score before split
  else
    score_best = loss(L_loss_state);

  double value_best = 0;
  size_t dim_best = 0;

  // node info
  printf("  mse = %.8f; samples = %Iu\n", score_best / len_ids, len_ids);

  // for every possible split, evaluate the score of split
  // bool split_ok = false;
  for (size_t feat_dim = 0; feat_dim < dim_features; feat_dim++){ // 2.1 choose feature dimension to split
    // set range for the feat_dim
    printf("    feat_dim: %Iu\n", feat_dim);
    size_t slice_start = slice_ranges[node_id][feat_dim][0];
    size_t slice_end = slice_ranges[node_id][feat_dim][1];
    
    for (size_t slice = slice_start; slice < slice_end; slice++) { // 2.2 choose a split and evaluate the score of the split
      // find split (-> modifies LR_flags)
      gen_LR_diff(samples, sample_ids[node_id], feat_dim, thresholds[feat_dim][slice], len_ids, LR_flags, LR_diff); // 'LR_flags' is allocated by main function, modified by this function
      
      // NOTE: eval_split relies on the shuffled samples (no need to specify feat_dim)
      double score = eval_split_diff(samples, sample_ids[node_id], LR_diff, &L_loss_state, &R_loss_state, len_ids);

      // update the score when it is better
      if (score < score_best) {
        score_best = score;
        slice_best = slice;
	      //value_best = samples[slice].features[feat_dim];
        
        dim_best = feat_dim;
        value_best = thresholds[dim_best][slice_best];

        L_loss_state_best = L_loss_state;
        R_loss_state_best = R_loss_state;
        //LR_flags[node_id]
	      //memcpy(LR_best, LR_flags, len_ids * sizeof(enum LR_flag));
      }
    }
  }
  /*if (!split_ok) {
    terminalize(&(tree[node_id]), samples, len_entire_samples, sample_ids[node_id], len_ids);
    printf("split ok did not become true.  Something is going wrong.");
    return;
  }*/

  /* 3. RECORD BEST-SPLIT-INFO ON NODE */
  
  tree[node_id].feature = dim_best;
  tree[node_id].value = value_best;
  tree[node_id].is_terminal = false; // set current node (leaf) values

  printf("  -> threshold: %.3f\n", value_best);

  /* 4. RECURSION: 'grow_tree' against child nodes */

  // LEFT: samples[slice_start:slice_best]
  size_t id_L = tree[node_id].id_left;
  slice_ranges[id_L][dim_best][0] = slice_ranges[node_id][dim_best][0];
  slice_ranges[id_L][dim_best][1] = slice_best;

  gen_LR(samples, sample_ids[node_id], dim_best, thresholds[dim_best][slice_best], len_ids, LR_flags);
  size_t len_left = flag2id(LR_flags, left, sample_ids, id_L, node_id, len_ids); // sample_ids -> array of left ids
  lossState init_state = { .sum = 0,.sqsum = 0,.len = 0 };
  init_LR_diff(LR_diff, len_entire_samples);
  grow_tree(tree, id_L, samples, sample_ids, LR_flags, LR_diff, thresholds, slice_ranges, len_entire_samples, len_left, dim_features, L_loss_state_best, init_state);

  // RIGHT: samples[right_slice_start:right_slice_end]
  size_t id_R = tree[node_id].id_right;
  slice_ranges[id_R][dim_best][0] = slice_best;
  slice_ranges[id_R][dim_best][1] = slice_ranges[node_id][dim_best][1];
  if (node_id == 2) {
    printf("DEBUG POINT\n");
  }
  gen_LR(samples, sample_ids[node_id], dim_best, thresholds[dim_best][slice_best], len_ids, LR_flags);
  size_t len_right = flag2id(LR_flags, right, sample_ids, id_R, node_id, len_ids); // sample_ids -> array of right ids
  assert(len_left + len_right == len_ids);
  init_LR_diff(LR_diff, len_entire_samples);
  grow_tree(tree, id_R, samples, sample_ids, LR_flags, LR_diff, thresholds, slice_ranges, len_entire_samples, len_right, dim_features, init_state, R_loss_state_best);
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

bool gen_LR(const sample* samples, size_t* sample_ids, size_t dim, double threshold, size_t len, enum LR_flag* LR){
  /// flag left or right for each sample (regardless of whether 'samples' is sorted)
  size_t L_count = 0;
  size_t R_count = 0;
  for (size_t n = 0; n < len; n++){
    size_t sample_id = sample_ids[n];
    if (samples[sample_id].features[dim] < threshold) {
      LR[sample_id] = left;
      L_count++;
    } else {
      LR[sample_id] = right;
      R_count++;
    }
  }
  if ((L_count < MIN_SAMPLES) || (R_count < MIN_SAMPLES))
    return false;
  else
    return true;
}

bool gen_LR_diff(const sample* samples, size_t* sample_ids, size_t dim, double threshold, size_t len, enum LR_flag* LR, enum LR_flag* LR_diff){
  /// flag left or right for each sample (regardless of whether 'samples' is sorted)
  size_t L_count = 0;
  size_t R_count = 0;
  for (size_t n = 0; n < len; n++){
    size_t sample_id = sample_ids[n];
    if (samples[sample_id].features[dim] < threshold) {
      // set LR difference (L changes to R then LR_diff holds "R", the converse holds)
      L_count++;
      if (LR[sample_id] == left) {// if previous flag is also left
	      LR_diff[sample_id] = none; // no difference
      } else { // if previous flag was not left (i.e. right)
	      LR_diff[sample_id] = left;
	      LR[sample_id] = left; // update LR
      }
    } else {
      R_count++;
      if (LR[sample_id] == right) {// if previous flag is also right
	      LR_diff[sample_id] = none; // no difference
      } else { // if previous flag was not right (i.e. left)
	      LR_diff[sample_id] = right;
	      LR[sample_id] = right; // update LR
      }
    }
  }
  if ((L_count < MIN_SAMPLES) || (R_count < MIN_SAMPLES))
    return false;
  else
    return true;
}



size_t flag2id(const enum LR_flag* LR_list, enum LR_flag flag_to_convert, size_t** sample_ids, size_t child_node_id, size_t parent_node_id, size_t len_ids){
  size_t count = 0;
  for (size_t i = 0; i < len_ids; i++){
    if (LR_list[sample_ids[parent_node_id][i]] == flag_to_convert){
      sample_ids[child_node_id][i] = sample_ids[parent_node_id][i];
      count++;
    } else {
      sample_ids[child_node_id][i] = INVALID_ID;
    }
  }
  qsort(sample_ids[child_node_id], len_ids, sizeof(size_t), comp_id);
  return count;
}

void init_ids(size_t** ids, size_t num_nodes, size_t len){
  for (size_t node_id = 0; node_id < num_nodes; node_id++) {
    for (size_t i = 0; i<len; i++) {
      ids[node_id][i] = i;
    }
  }
  
}

void get_thresholds(sample* samples, sample* smpl_buffer, double** thresholds, size_t*** ranges, size_t n_features, size_t len_samples) {
  /* TODO: IMPLEMENT */
  memcpy(smpl_buffer, samples, len_samples); // keep original 'samples'
  for (size_t feat_dim = 0; feat_dim < n_features; feat_dim++) {
    // 1. sort
    extern size_t comp_feat_dim;
    comp_feat_dim = feat_dim;
    qsort(smpl_buffer, len_samples, sizeof(sample), comp_sample);

    // 2. record elements
    size_t n_unique_elements = 1; // init

                                  // size_t i = 0;
    thresholds[feat_dim][0] = smpl_buffer[0].features[feat_dim]; // the first element is always unique
    thresholds[feat_dim][0] = 0;
    for (size_t i = 1; i < len_samples; i++) {
      if (smpl_buffer[i - 1].features[feat_dim] != smpl_buffer[i].features[feat_dim]) {
        thresholds[feat_dim][n_unique_elements] = smpl_buffer[i].features[feat_dim];
        n_unique_elements++;
      }
    }
    // set ranges for each thresholds
    // i.e. ranges[feat_dim] = range(0, n_unique_elements)
    for (size_t node_id = 0; node_id < NUM_NODES; node_id++) { // all node initially have the same ranges.
      ranges[node_id][feat_dim][0] = 0;
      ranges[node_id][feat_dim][1] = n_unique_elements;
    }    
  }
}

double loss(lossState ls) {
  if (ls.len > 0)
    return ls.sqsum - pow2(ls.sum) / ls.len;
  else
    return -1; // loss cannot be calculated
}

void init_LR_diff(enum LR_flag* LR_diff, size_t len_entire_samples) {
  for (size_t i = 0; i < len_entire_samples; i++) {
    LR_diff[i] = none;
  }
}