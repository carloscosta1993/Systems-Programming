#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t p;
    int i, n = 10, j = 1, status, x;
    for(i = 0; i < n; i++){
        p = fork();
        srand(getpid());
        int randomnumber = rand() % 10;
        if(p == 0){
            printf("Child: %d, iteration: %d, time sleep:%d\n",getpid(), i, randomnumber);
            sleep(randomnumber);
            x = 11;
            exit(0);
        }/*else{
            printf("Parent process started\n");
            if ((p = wait(&status)) == -1){
                perror("wait error");
            }else{
                if (WIFSIGNALED(status) != 0)
                    printf("Child process ended because of signal %d\n", WTERMSIG(status));
                else if (WIFEXITED(status) != 0)
                    printf("Child process ended normally; status = %d\n", WEXITSTATUS(status));
                else
                    printf("Child process did not end normally\n");
            }
        }*/
    }

    while (wait(NULL) > 0)
    {
        printf("%d child completed %i\n", j++, p);
    }

    printf("all children terminated. in parent now\n");
    return 0;
}