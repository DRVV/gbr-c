#include <stdio.h>

int factorial(int);

int main(){
  
}

int factorial(int n){
  if (n == 1){
    return n;
  } else {
    return n * factorial(n-1);
  }
}
