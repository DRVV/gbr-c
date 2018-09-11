#include <stdio.h>

int main(){
	printf("Hello, can you see me before you hit any keys?");
	fflush(stdout);
	getchar();
	printf("Right, your stream seems to be buffered.");
}
