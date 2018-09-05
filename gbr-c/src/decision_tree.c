/*
 * decision_tree.c
 *
 *  Created on: 2018/09/05
 *      Author: sysl1_1704a
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
// parameters
#define MAX_DEPTH 2
#define NUM_NODES 15 // 2^(MAX_DEPTH+1) - 1

#define DEFAULT_FEATURE -1
#define DEFAULT_VALUE -1.000

#define LEN_DATA

/* structs */

typedef struct node {
  size_t id;
  size_t id_left;
  size_t id_right;
  int feature;
  double value;
  bool is_terminal;
} node;
  
/* prototypes */

void make_nodes(size_t);

int find_left(int);
int find_right(int);
int find_depth(int);

double log2(double);
double var(double* arr, size_t, size_t);

int comparetor(const void*, const void*);

void grow_tree(node* tree, size_t, double* data, size_t, size_t);
bool grow_should_stop();

double eval_split(double* arr, size_t, size_t, size_t);

void print_tree(node tree[], size_t);
void print_double_array(double arr[], size_t);

node tree[NUM_NODES];
// main

int main() {
  // dummy data
  double input_data[] = {3,1,4,1,5,9,2,6,5,3,5,9};
  int len_data = 12;
  //print_double_array(input_data, len_data);

  // initialise nodes
  size_t i = 0;
  for (i=0; i<NUM_NODES; i++) {
    tree[i].id = i;
    tree[i].id_left = find_left(i);
    tree[i].id_right = find_right(i);
    tree[i].feature = DEFAULT_FEATURE;
    tree[i].value = DEFAULT_VALUE;
    if (find_depth(i) < MAX_DEPTH) {
      tree[i].is_terminal = false;
    } else {
      tree[i].is_terminal = true;
    }
  }
  
  ///// debug /////
  print_tree(tree, NUM_NODES);

  /* // sort input data */
  qsort(input_data, len_data, sizeof(double), comparetor);
  // print_double_array(input_data, len_data);

  grow_tree(tree, 0, input_data, 0, len_data-1);

  print_tree(tree, NUM_NODES);
  /* // fit tree to input data */

  /* grow_tree(*tree, 0, *input_data, 0); // the initial node is 0, and the inital index is 0. */
  
  /* // output result tree data */
  /* print_tree(tree); */
}

double eval_split(double* arr, size_t idx_start, size_t idx_slice, size_t idx_end){
  /* calculate each variance*/
  // NOTE: use slice convention
  double score_left = var(arr, idx_start, idx_slice-1); // arr[idx_start:idx_slice]
  double score_right = var(arr, idx_slice, idx_end); // arr[idx_slice: idx_end]
  // total
  return (score_left + score_right);
}


void grow_tree(node* tree, size_t node_id, double* arr, size_t idx_start, size_t idx_end){
  //
  // find best split
  //
  
  // init
  size_t idx_best = idx_start;
  double score_best = (float)INT_MAX;
  
  // for every possible split, evaluate the score of split
  size_t slice;
  for (slice=idx_start; slice <= idx_end; slice++) {
    printf("current best score: %.3f\n", score_best);
    double score = eval_split(arr ,idx_start, slice, idx_end);
    if (score < score_best){
      // update if current score wins
      score_best = score;
      idx_best = slice;
    }
  }
  // set node values
  tree[node_id].feature = 0; // TODO: special for single feature input
  tree[node_id].value = idx_best;
  
  // check stopping condition

  if (grow_should_stop(find_depth(node_id))){
    // print_tree(tree, NUM_NODES);
    return;
  }
  else{
    size_t left_idx_start = idx_start;
    size_t left_idx_end = idx_best;
    size_t right_idx_start = idx_best+1;
    size_t right_idx_end = idx_end;
    
    grow_tree(tree, tree[node_id].id_left, arr, left_idx_start, left_idx_end);
    grow_tree(tree, tree[node_id].id_right, arr, right_idx_start, right_idx_end);
  }
      
  // RECURSION: grow_tree against child nodes
  
}
bool grow_should_stop(int depth){
  if (depth > MAX_DEPTH)
    return true;
  else
    return false;
}  

void print_tree(node tree[], size_t len){
  size_t i;
  printf("{tree: \n[");
  for (i=0; i < len; i++){
    printf("{id: %d, id_left: %d, id_right: %d, feature: %d, value: %.3f, is_terminal: %d}",
	   tree[i].id,
	   tree[i].id_left,
	   tree[i].id_right,
	   tree[i].feature,
	   tree[i].value,
	   tree[i].is_terminal);
    // final comma should not appear
    if (i==len-1){
      printf("\n");
    } else {
      printf(",\n");
    }
  }
  printf("]}\n");
}

void print_double_array(double arr[], size_t len){
  size_t i;
  for (i=0; i < len; i++){
    printf("%.3f, ",(double)arr[i]);
  }
  printf("\n");
}

// for qsort
int comparetor (const void* a, const void* b){
  double* aa =(double*)a;
  double* bb =(double*)b;
  
  if (*aa > *bb)
    return 1;
  else if (*aa < *bb)
    return -1;
  else
    return 0;
}


int find_left(int i){
  return 2 * i + 1;
}

int find_right(int i){
  return 2 * i + 2;
}

int find_depth(int i){
  return (int)floor(log2(i+1));
}

double log2(double x){
  return log(x) / log(2);
}  

double var(double* arr, size_t idx_start, size_t idx_end){
  int len = idx_end - idx_start + 1;
  if (len < 2){
    printf("Array length is less than 2.  Cannot calculate variance.\n");
    return INT_MAX;
  }
  double squared=0;
  double mean=0;
  
  size_t i;
  for (i=idx_start; i <= idx_end; i++){
    //printf("%.3f\n", arr[i]);
    squared += pow(arr[i], 2);
    mean += arr[i];
  }
  mean = mean / len;
  // <X^2> - <X>^2
  double squared_mean = squared / len;
  double mean_squared = pow(mean, 2);
  return (squared_mean - mean_squared);
}
