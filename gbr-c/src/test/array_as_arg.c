#include <stdio.h>

void repl(int arr[], size_t);

int main(){
  int arr[] = {3, 1, 4, 1, 5};
  int target_idx = 2;
  printf("original: %d\n", arr[target_idx]);

  repl(arr, target_idx);
  
  printf("replaced: %d\n", arr[target_idx]);

}


void repl(int arr[], size_t target){
  arr[target] = 10000;
}
