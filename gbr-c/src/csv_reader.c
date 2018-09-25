/* 'malloc' is used */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "csv_reader.h"

shape shape_of(FILE* fp){
  shape csv_shape;
  size_t n_lines = 0;
  size_t n_token = 0;
  
  char line[BUFFER_SIZE];
  while (fgets(line, BUFFER_SIZE, fp)){
    n_lines++;
    char* tmp = strdup(line);
    char* token = strtok(tmp, ",");
    
    n_token = 0; // init n_token to count cols
    while (token != NULL) {
      token = strtok(NULL, ",");
      n_token++;
    }
    free(tmp);
  }
  csv_shape.n_rows = n_lines;
  csv_shape.n_columns = n_token;
  
  return csv_shape;
}

void print_pparr(double** arr, size_t n_rows, size_t n_cols){
  size_t row, col;
  printf("[");
  for (row = 0; row < n_rows; row++){
    printf("[");
    for (col = 0; col < n_cols; col++){
      printf("%.3f, ", arr[row][col]);
    }
    printf("\b\b");
    printf("],\n");
  }
  printf("]\n");
}

double** read_csv(char* path){
  // check file existence
  if (access(path, F_OK) == -1){
    printf("File not found.  Abort.");
    exit(EXIT_FAILURE);
  }
  // find shape
  FILE* fp = fopen(path, "r");
  shape shp = shape_of(fp);
  fclose(fp);
  size_t n_rows = shp.n_rows;
  size_t n_cols = shp.n_columns;

  // init memory
  double** data = malloc(n_rows * sizeof(double *));
  printf("malloc shape: (%Iu, %Iu)\n", n_rows, n_cols);

  // read contents
  fp = fopen(path, "r");
  size_t row = 0;
  char line[BUFFER_SIZE];
  while (fgets(line, BUFFER_SIZE, fp)){
    char* tmp = strdup(line);
    char* token = strtok(tmp, ",");

    data[row] = malloc(n_cols * sizeof(double));
    
    // read line
    size_t col = 0;
    while (token != NULL) {
      data[row][col] = atof(token);
      token = strtok(NULL, ",");
      col++;
    }
    free(tmp);
    row++;
  }
  assert(row == n_rows);
  fclose(fp);

  return data;
}

sample* make_samples_from_csv(char* path){
  // check file existence
  if (access(path, F_OK) == -1){
    printf("File not found.  Abort.");
    exit(EXIT_FAILURE);
  }
  // find shape
  FILE* fp = fopen(path, "r");
  shape shp = shape_of(fp);
  fclose(fp);
  size_t n_rows = shp.n_rows;
  size_t n_cols = shp.n_columns;

  // init memory
  sample* smp = malloc(n_rows * sizeof(sample));
  printf("malloc shape: (%Iu, %Iu)\n", n_rows, n_cols);

  // read contents
  fp = fopen(path, "r");
  size_t row = 0;
  char line[BUFFER_SIZE];
  while (fgets(line, BUFFER_SIZE, fp)){
    char* tmp = strdup(line);
    char* token = strtok(tmp, ",");

    //data[row] = malloc(n_cols * sizeof(double));
    // read line
    size_t col = 0;
    smp[row].features = malloc((n_cols-N_TARGETS) * sizeof(double));

    while ((token != NULL) && (col < n_cols - N_TARGETS)) {
      smp[row].features[col] = atof(token);
      token = strtok(NULL, ",");
      col++;
    }
    smp[row].target = atof(token);
    free(tmp);
    row++;
      
  }
  assert(row == n_rows);
  fclose(fp);

  return smp;
}
