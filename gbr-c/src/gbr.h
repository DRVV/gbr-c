#ifndef GBR_H
#define GBR_H

#include <stddef.h>
#include "sample.h"
#include "decision_tree.h"

#define BUFFER_SIZE 1024

#define N_ESTIMATORS 100
#define NUM_TREES N_ESTIMATORS

#define NUM_FEATURES 2
#define NUM_TARGETS 1

void init_forest(node forest[][NUM_NODES], size_t, size_t);
void print_forest(node forest[][NUM_NODES], size_t, size_t);

//void gbr_predict(node** forest, size_t n_trees, size_t n_nodes, double** features, double* result, size_t len_data);
//void gbr_fit(node** forest, size_t n_trees, size_t n_nodes, sample* residual_samples, size_t len_data);
void gbr_fit(node forest[][NUM_TREES], size_t n_trees, size_t n_nodes, sample* residual_samples, sample* training_samples, size_t len_data, double* pred, double* pred_by_each_tree, sample* residual_samples_cp, double** features, size_t n_features);
void gbr_predict(node forest[][NUM_TREES], size_t n_trees, size_t n_nodes, double** features, double* result, double* result_delta, size_t len_data);

void get_residual(sample*, double*, sample*, size_t);
void update_prediction(double*, double*, size_t);
void update_residual(sample*, double*, sample*, size_t len_data);

#endif
