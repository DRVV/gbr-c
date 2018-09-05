#include <stdio.h>
#include <math.h>

double std(double* arr, size_t);

int main(){
  double arr[] = {3, 1, 4};
  printf("std: %.3f", std(arr, 3));
}

double std(double* arr, size_t len){
  double squared=0;
  double mean=0;
  
  size_t i;
  for (i=0; i < len; i++){
    squared += pow(arr[i], 2);
    mean += arr[i];
  }
  mean = mean / len;
  printf("mean: %.3f", mean);
  // <X^2> - <X>^2
  double squared_mean = squared / len;


  double mean_squared = pow(mean, 2);

  printf("squared_mean: %.3f", squared_mean);
  printf("mean_squared: %.3f", mean_squared);
  return (squared_mean - mean_squared);
}

