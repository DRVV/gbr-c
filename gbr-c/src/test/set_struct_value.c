#include <stdio.h>

typedef struct{
  int memb;
} tekito;

void modifier(tekito*, int);

int main(){
  tekito hoge;
  hoge.memb = 3;
  printf("content before: %d\n", hoge.memb);
  modifier(&hoge, 1000);
  printf("content after: %d\n", hoge.memb);
}

void modifier(tekito* m, int val){
  //printf("your input: %d\n", val);
  tekito holder = *m;
  holder.memb = val;
}
