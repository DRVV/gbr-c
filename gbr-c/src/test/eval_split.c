#include <stdio.h>
#include <math.h>


double eval_split(double* arr, size_t, size_t, size_t);
double var(double* arr, size_t, size_t);

int main(){
  double arr[] = {3, 1, 4, 1, 5, 9, 2};
  printf("central split: %.3f\n", eval_split(arr, 0, 3, 6));
  printf("next: %.3f\n", eval_split(arr, 3, 3+2, 6));
}



  
double eval_split(double* arr, size_t idx_start, size_t idx_slice, size_t idx_end){
  double score_left = var(arr, idx_start, idx_slice-1);
  double score_right = var(arr, idx_slice, idx_end);
  // total
  return (score_left + score_right);
}



double var(double* arr, size_t idx_start, size_t idx_end){
  int len = idx_end - idx_start + 1;
  if (len < 2){
    printf("Array length is less than 2.  Cannot calculate variance.");
    return -1;
  }
  double squared=0;
  double mean=0;
  
  size_t i;
  for (i=idx_start; i <= idx_end; i++){
    printf("%.3f\n", arr[i]);
    squared += pow(arr[i], 2);
    mean += arr[i];
  }
  mean = mean / len;
  // <X^2> - <X>^2
  double squared_mean = squared / len;
  double mean_squared = pow(mean, 2);
  return (squared_mean - mean_squared);
}
