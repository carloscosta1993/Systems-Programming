#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int main() {
    pid_t p;
    int i, n = 10;
    for(i = 0; i < n; i++){
        p = fork();
        srand(getpid());
        int randomnumber = rand() % 10;
        if(p == 0){
            printf("Child: %d, iteration: %d, time sleep:%d\n",getpid(), i, randomnumber);
            sleep(randomnumber);
            exit(0);
        }else{
            printf("Parent, iteration: %d\n",i);
        }
    }
    return 0;
}