#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* get_field(char* line, int num);
#define LEN 100

int main(int argc, char** argv){
  double x[LEN];
  double y[LEN];

  double data[2][LEN];

  FILE* stream = fopen(argv[1], "r");
  char line[1024];
  size_t i = 0;
  size_t j=0;
  while((fgets(line, 1024, stream)) && (j<LEN)){
    char* tmp = strdup(line);
    //printf("x: %s, y: %s\n", get_field(tmp, 1), get_field(tmp, 2));

    char* token = strtok(tmp, ",");

    while(token!=NULL){
      printf("i,j = %Iu, %Iu ", (i%2), j);      
      printf("value= %.3f\n", atof(token));

      data[(i%2)][j] = atof(token);

      token = strtok(NULL, ",");
      i++;
    }
    free(tmp);
    j++;
  }

  size_t ii;
  size_t jj;
  for (ii=0; ii<2; ii++){
    for (jj=0; jj<LEN; jj++){
      printf("%.2f ", data[ii][jj]);
    }
    printf("\n");
  }
}

const char* get_field(char* line, int num){
  const char* tok;
  for (tok = strtok(line, ","); tok && *tok; tok = strtok(NULL, ",\n")){
    if (!--num)
      return tok;
  }

  return NULL;
}
