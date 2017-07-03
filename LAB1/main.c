#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char * argv[]){
    printf("Hi");
    char *ch;
    int Tsize=0;
    int i, j;

    for(i=1; i<=argc; i++){
           Tsize = Tsize + strlen(argv[i]);
    }

    printf("%s",*ch);


    return 0;
}
