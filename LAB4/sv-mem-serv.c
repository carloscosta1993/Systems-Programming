#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SHMSZ     27

int main(){
    char c;
    int shmid;
    key_t key;
    int *shm, *s;

    /*
     * We'll name our shared memory segment
     * "5678".
     */
    key = 5678;

    /*
     * Create the segment.
     */
    shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }
    printf("%d\n", shmid);

    /*
     * Now we attach the segment to our data space.
     */
    shm = shmat(shmid, NULL, 0)
    if (shm == (int *) -1) {
        perror("shmat");
        exit(1);
    }

	int i = 1;
	while(1){
		*shm = i++;
	}
	shmctl(shmid,  IPC_RMID , NULL);

    exit(0);
}
