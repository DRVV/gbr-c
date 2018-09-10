#include <stdio.h>
#include <string.h>
#define ARRSIZE 5
void print_double_array(double*, size_t);

int main(){
  double arr[ARRSIZE] = {3,1,4,1,5};
  double arr_target[ARRSIZE] = {0};

  print_double_array(arr, ARRSIZE);
  print_double_array(arr_target, ARRSIZE);

  memcpy(arr_target, arr, sizeof(double)*ARRSIZE);
  print_double_array(arr_target, ARRSIZE);
}

void print_double_array(double arr[], size_t len) {
	size_t i;
	for (i = 0; i < len; i++) {
		printf("%.3f, ", (double) arr[i]);
	}
	printf("\n");
}
