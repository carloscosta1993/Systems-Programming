#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#define childs 3
#define MAX 30

typedef struct{
    unsigned  start, end;
    unsigned int  m_7, m_19, m_7_19;

} com_struct;

int main(){

    int file;
    pid_t p;
    com_struct *structure;
    file=shm_open("/Ex3.txt", O_CREAT | O_RDWR, 0600);
    ftruncate(file, childs*sizeof(com_struct));
    structure = mmap(NULL, childs*sizeof(com_struct), PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);


    for (unsigned int  j = 0  ; j < childs ; j++) {
        structure[j].start = MAX / childs * j;
        structure[j].end = MAX / childs * (j + 1);
        structure[j].m_7_19 = 0;
        structure[j].m_19 = 0;
        structure[j].m_7 = 0;
    }
    for (int s =0; s<childs; s++) {
        p=fork();
        for (unsigned int i =structure[s].start; i <structure[s].end; i++) {
            if (i % 7 == 0) {
                structure[s].m_7++;
            }
            if (i % 19 == 0) {
                structure[s].m_19++;
            }
            if ((i % 7 == 0) && (i % 19 == 0)) {
                structure[s].m_7_19++;
            }

        }
        if(p == 0){
        exit(0);}
    }

    wait(NULL);


    for (int s =0; s<childs; s++) {
        printf("m 7    %d\n", structure[s].m_7);
        printf("m   19 %d\n", structure[s].m_19);
        printf("m 7 19 %d\n", structure[s].m_7_19);
    }

}