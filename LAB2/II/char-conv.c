#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


int main(){
	char v1[100];
	char *v2;
	int i;

    v2 = malloc(10);

	printf("Write a word");
	fgets(v1, 100, stdin);

	for (i=0; v1[i]!=0; i+1){
		v2[i] = toupper(v1[i]);
	}

	printf("Converted string: %s", v2);
    v2 ='\0';

}