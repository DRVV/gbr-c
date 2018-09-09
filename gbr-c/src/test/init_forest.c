#include <stdio.h>
#include "../decision_tree_unix.h"
#define NUM_TREES 2


void init_forest(node forest[][NUM_NODES], size_t, size_t);

void print_forest(node forest[][NUM_NODES], size_t, size_t);

int main(){

  node forest[NUM_TREES][NUM_NODES];
  init_forest(forest, NUM_TREES, NUM_NODES);
  print_forest(forest, NUM_TREES, NUM_NODES);
}

void init_forest(node forest[][NUM_NODES], size_t num_trees, size_t num_nodes){
  size_t i;
  for (i = 0; i < num_trees; i++) {
    init_tree(forest[i], num_nodes);
  }
}

void print_forest(node forest[][NUM_NODES], size_t num_trees, size_t num_nodes){
  size_t i;
  for (i = 0; i < num_trees; i++) {
    printf("tree number %zu\n", i);
    print_tree(forest[i], num_nodes);
  }
}
