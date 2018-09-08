#include <stdio.h>

typedef struct {
  int x,y;
} pos;

void f(pos*, int, int);
void g(pos*, int, int);
int main(){
  pos arr[3];
  f(arr, 100, 200);
  printf("arr[0] x, y:  %d, %d \n", arr[0].x, arr[0].y);

  g(arr, 300, 400);

  printf("arr[0] x, y:  %d, %d \n", arr[0].x, arr[0].y);
}

void f(pos* arr, int valx, int valy){
  arr[0].x = valx;
  arr[0].y = valy;
  //arr[2].x = val;
}

void g(pos* arr, int valx, int valy){
  pos* poi = &(arr[0]);
  (*poi).x = valx;
  (*poi).y = valy;
}
