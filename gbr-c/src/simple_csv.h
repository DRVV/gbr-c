/*
 * simple_csv.h
 *
 *  Created on: 2018/09/11
 *      Author: sysl1_1704a
 */

#ifndef SIMPLE_CSV_H_
#define SIMPLE_CSV_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* get_field(char* line, int num);
void read_csv(int, int, char*, double[100][2]);

#define SEPARATOR ","
#define LINE_TERMINATOR ",\n"

const char* get_field(char*line, int num){
	const char* tok;
	for (tok = strtok(line, SEPARATOR); tok && *tok; tok = strtok(NULL, LINE_TERMINATOR)){
		if(!--num)
			return tok;
	}
	return NULL;
}


void read_csv(int row, int col, char *filename, double data[100][2]){
	FILE *file;
	file = fopen(filename, "r");

	int i = 0;
    char line[4098];
	while (fgets(line, 4098, file) && (i < row))
    {
    	// double row[ssParams->nreal + 1];
        char* tmp = strdup(line);

	    int j = 0;
	    const char* tok;
	    for (tok = strtok(line, "\t"); tok && *tok; j++, tok = strtok(NULL, "\t\n"))
	    {
	        data[i][j] = atof(tok);
	        printf("%f\t", data[i][j]);
	    }
	    printf("\n");

        free(tmp);
        i++;
    }
}



#endif /* SIMPLE_CSV_H_ */
