#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* get_field(char* line, int num);

int main(int argc, char** argv){
	FILE* stream = fopen(argv[1], "r");
	
	char line[1024];
	while(fgets(line, 1024, stream)){
		char* tmp = strdup(line);
		//printf("x: %s, y: %s\n", get_field(tmp, 1), get_field(tmp, 2));
		printf("x: %s\n", get_field(tmp, 1));
		printf("y: %s\n", get_field(tmp, 2));
		free(tmp);
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
