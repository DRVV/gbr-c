#include <stdio.h>
#define ARRSIZE 5
void print_double_array(double*, size_t);


int main(){
  double arr[ARRSIZE] = {0};
  print_double_array(arr, ARRSIZE);

  // no init
  double arrnoinit[ARRSIZE];
  print_double_array(arrnoinit, ARRSIZE);
}

void print_double_array(double arr[], size_t len) {
	size_t i;
	for (i = 0; i < len; i++) {
		printf("%.3f, ", (double) arr[i]);
	}
	printf("\n");
}
