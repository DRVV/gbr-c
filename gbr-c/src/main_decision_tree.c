#include <stdio.h>
#include "decision_tree.h"

int main(int argc, char** argv) {
  
  char* path = argv[1];
  FILE* fp = fopen(path, "r");
  shape shp = shape_of(fp);

  size_t n_rows = shp.n_rows;
  size_t n_cols = shp.n_columns;
  fclose(fp);

  size_t len_data = n_rows;

  int n_features = n_cols - DIM_TARGETS;
  sample* training_samples = make_samples_from_csv(path);
  print_samples(training_samples, n_rows, n_features);

  double** input_features = malloc(n_rows * sizeof(double*));
  get_features(training_samples, n_rows, input_features);

  // initialise nodes
  node tree[NUM_NODES];
  init_tree(tree, NUM_NODES);

  ///// debug /////
  puts("Init nodes:");
  print_tree(tree, NUM_NODES);

  puts("Fitting...");
  fit(tree, training_samples, len_data, n_features);
  // sort input data

  /* output result tree data */
  puts("Fit result:");
  print_tree(tree, NUM_NODES);

  halt();

  puts("Prediction");
  double* result = malloc(n_rows * sizeof(double));
  
  predict(tree, NUM_NODES, input_features, result, len_data);

  print_double_array(result, len_data);
  return 0;
}
