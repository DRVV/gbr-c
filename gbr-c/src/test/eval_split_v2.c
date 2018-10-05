#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "../sample.h"
#include <limits.h>

#define LEN 10
#define pow2(x) x*x
#define NA (size_t)INT_MAX

enum LR_flag {left = 0, right = 1, none = 2};

double eval_split(sample*, size_t*, enum LR_flag*, size_t);
void print_arr(size_t* arr, size_t len);
void init_ids(size_t*, size_t);

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

  printf("L: count, mean, squared sum: %.d, %.3f, %.3f\n", L_count, L_mean, L_squared_sum);
  printf("R: count, mean, squared sum: %.d, %.3f, %.3f\n", R_count, R_mean, R_squared_sum);

  
  // total score
  return L_score + R_score;
}

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}


int main(){
  sample samples[LEN];
  for (size_t i=0; i<LEN; i++){
    samples[i].features = calloc(sizeof(double), 5);
    samples[i].target = i;
  }

  size_t ids[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  enum LR_flag LR_flags[] = {left, right, right, left, left, none, none, none, none, none};

  int len = 5;
  
  double score = eval_split(samples, ids, LR_flags, len);
  printf("split score: %.3f\n", score);

  /* qsort(LR_flags, len, sizeof(enum LR_flag), cmpfunc); */

  /* for (size_t j=0; j<LEN; j++){ */
  /*   printf("sorted flags: %d\n", LR_flags[j]); */
  /* } */

  size_t L_ids[LEN];
  size_t R_ids[LEN];

  init_ids(L_ids, LEN);
  init_ids(R_ids, LEN);
  
  print_arr(L_ids, LEN);
  
  size_t L_len = 0;
  size_t R_len = 0;
  
  for (size_t i = 0; i < len; i++){
    if (LR_flags[i] == left){
      L_len++;
      printf("i: %Iu, id: %Iu\n", i, ids[i]);
      L_ids[i] = ids[i];
      R_ids[i] = NA;
    } else {
      R_len++;
      L_ids[i] = NA;
      R_ids[i] = ids[i];
    }
  }
  printf("Left:\n");
  print_arr(L_ids, LEN);

  qsort(L_ids, LEN, sizeof(size_t), cmpfunc);
  qsort(R_ids, LEN, sizeof(size_t), cmpfunc);

  printf("Left:\n");
  print_arr(L_ids, L_len);

  printf("Right:\n");
  print_arr(R_ids, R_len);
  
  enum LR_flag L_flags[] = {left, right};
  enum LR_flag R_flags[] = {right, left};
  
  double L_score = eval_split(samples, L_ids, L_flags, L_len);
  printf("split score: %.3f\n", L_score);
}

void print_arr(size_t* arr, size_t len){
  for (size_t i=0; i<len; i++){
    printf("%Iu\n", arr[i]);
  }
}

void init_ids(size_t* ids, size_t len){
  for (size_t i = 0; i < len; i++){
    ids[i] = (size_t)INT_MAX;
  }
}
