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

// comparetor
int comparetor(const void*, const void*);
int comp_sample(const void*, const void*);

// tree related
void init_tree(node* tree, size_t);
void grow_tree(node* tree, size_t, sample* data, size_t, size_t);
bool grow_should_stop(size_t, size_t, size_t);

double eval_split(sample* arr, size_t, size_t, size_t);

void predict(node* tree, size_t, double* predictor, double* result, size_t);
double trace_tree(node* tree, size_t, double predictor);

void print_tree(node tree[], size_t);
void print_double_array(double arr[], size_t);

void print_samples(sample samples[], size_t);

void create_training_samples(double*, double*, sample*, size_t);

void terminalize(node*, sample*, size_t, size_t);


#define LEN_DATA 12
int main() {
	// dummy data
	double input_feature[] = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 9 };
	double input_target[] = { 30, 10, 40, 10, 50, 90, 20, 60, 50, 30, 50, 90 };
	int len_data = 12;

	sample training_samples[LEN_DATA];
	create_training_samples(input_feature, input_target, training_samples,
			LEN_DATA);
	print_samples(training_samples, LEN_DATA);
	//print_double_array(input_data, len_data);

	// initialise nodes
	node tree[NUM_NODES];
	init_tree(tree, NUM_NODES);

	///// debug /////
	puts("Init nodes:");
	print_tree(tree, NUM_NODES);

	/* // sort input data */
	qsort(training_samples, LEN_DATA, sizeof(sample), comp_sample);

	/* fit tree to input data */
	puts("Fitting...");
	grow_tree(tree, 0, training_samples, 0, len_data - 1); // the initial node is 0, and the inital index is 0.

	/* output result tree data */
	puts("Fit result:");
	print_tree(tree, NUM_NODES);

	puts("Prediction");
	double result[LEN_DATA];
	predict(tree, NUM_NODES, input_feature, result, len_data);

	print_double_array(result, len_data);

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

double eval_split(sample* arr, size_t idx_start, size_t idx_slice,
		size_t idx_end) {
	/* calculate each variance*/
	// NOTE: use slice convention
	double score_left = var_target(arr, idx_start, idx_slice - 1); // arr[idx_start:idx_slice]
	double score_right = var_target(arr, idx_slice, idx_end); // arr[idx_slice: idx_end]
	// total
	return (score_left + score_right);
}

void grow_tree(node* tree, size_t node_id, sample* arr, size_t idx_start,
		size_t idx_end) {

	// node* current_node = &tree[node_id];
	// check stopping condition
	if (grow_should_stop(node_id, idx_start, idx_end)) {
		terminalize(&(tree[node_id]), arr, idx_start, idx_end);
		return;
	} else {
		/* find best split */
		// init
		size_t idx_best = idx_start;
		double score_best = (float) INT_MAX;

		// for every possible split, evaluate the score of split
		size_t slice;
		for (slice = idx_start; slice <= idx_end; slice++) {
			printf("current best score: %.3f\n", score_best);
			double score = eval_split(arr, idx_start, slice, idx_end);
			if (score < score_best) {
				// update if current score wins
				score_best = score;
				idx_best = slice;
			}
		}
		size_t left_idx_start = idx_start;
		size_t left_idx_end = idx_best - 1;
		size_t right_idx_start = idx_best;
		size_t right_idx_end = idx_end;

		// another stopping condition
		if (left_idx_start == left_idx_end){
			terminalize(&(tree[node_id]), arr, idx_start, idx_end);
			return;
		} else {
			// set current node (leaf) values
			tree[node_id].feature = 0; // TODO: special for single feature input
			tree[node_id].value = arr[idx_best].feature;
			tree[node_id].is_terminal = false;

			// RECURSION: apply 'grow_tree' against child nodes

			grow_tree(tree, tree[node_id].id_left, arr, left_idx_start,
					left_idx_end);
			grow_tree(tree, tree[node_id].id_right, arr, right_idx_start,
					right_idx_end);
		}
	}
}

bool grow_should_stop(size_t node_id, size_t idx_start, size_t idx_end) {
	// stopping conditions
	bool chk_depth = (find_depth(node_id) > MAX_DEPTH); // depth exceeds max

	int len = idx_end - idx_start + 1;
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
		printf("%.3f, ", (double) arr[i]);
	}
	printf("\n");
}

void print_samples(sample samples[], size_t len) {
	size_t i;
	printf("(feature, target)\n");
	for (i = 0; i < len; i++) {
		printf("(%.3f, %.3f)\n", samples[i].feature, samples[i].target);
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

	if (aa.target > bb.target)
		return 1;
	else if (aa.target < bb.target)
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

double mean(double* arr, size_t idx_start, size_t idx_end) {
	double mean = 0;
	double len = idx_end - idx_start + 1;
	size_t i = 0;
	for (i = idx_start; i <= idx_end; i++) {
		mean += arr[i];
	}
	mean = mean / len;
	return mean;
}

double mean_target(sample* arr, size_t idx_start, size_t idx_end) {
	double mean = 0;
	double len = idx_end - idx_start + 1;
	size_t i = 0;
	for (i = idx_start; i <= idx_end; i++) {
		mean += arr[i].target;
	}
	mean = mean / len;
	return mean;
}

double var_target(sample* arr, size_t idx_start, size_t idx_end) {
	int len = idx_end - idx_start + 1;
	if (len < 2) {
		printf("Array length is less than 2.  Cannot calculate variance.\n");
		return arr[0].target;
	}
	double squared = 0;
	double mean = 0;

	size_t i;
	for (i = idx_start; i <= idx_end; i++) {
		//printf("%.3f\n", arr[i]);
		squared += pow(arr[i].target, 2);
		mean += arr[i].target;
	}
	mean = mean / len;
	// <X^2> - <X>^2
	double squared_mean = squared / len;
	double mean_squared = pow(mean, 2);
	return (squared_mean - mean_squared);
}

void terminalize(node* target_node, sample* arr, size_t idx_start, size_t idx_end){
	(*target_node).is_terminal = true;
	(*target_node).value = mean_target(arr, idx_start, idx_end);
}
