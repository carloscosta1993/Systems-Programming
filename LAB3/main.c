#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int main() {
    int i;
    pid_t pid;
    int randomnumber;
    int time_sleep = 0;
    for(i = 0; i < 10; i++) {
        pid = fork();
        srand(getpid());
        randomnumber = rand() % 10;
        if(pid < 0) {
            printf("Error");
            exit(1);
        } else if (pid == 0) {
            sleep(1);
            printf("Child (%d): pid: %d | time sleep:%d \n", i + 1, getpid(), randomnumber);
            exit(0);
        } else  {
            time_sleep = time_sleep + randomnumber;
            wait(NULL);
        }
    }
    printf("PARENT: %d, TOTAL TIME ASLEEP: %d\n", getpid(), time_sleep);

    return 0;
}
