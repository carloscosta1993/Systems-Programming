#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#define child_processes 3
#define MAX 30

typedef struct{
		unsigned start, end;
		unsigned m_7, m_19, m_7_19;
} com_struct;

int main(){
	com_struct *limits;
	pid_t pid, wpid;
	int status = 0;
	int total_m_7 = 0, total_m_19 = 0, total_m_7_19 = 0;
	int file = open("ex_3.txt",  O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    ftruncate(file, child_processes*sizeof(com_struct));
    limits = mmap(NULL, child_processes*sizeof(com_struct), PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
	//limits = mmap(NULL, child_processes*sizeof(com_struct), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON,-1, 0);
	
	for (unsigned int i = 0; i < child_processes ; i++){
		limits[i].start = ((MAX/child_processes)*i) + 1;
		limits[i].end = (MAX/child_processes)*(i+1);
		limits[i].m_7 = 0;
		limits[i].m_19 = 0;
		limits[i].m_7_19 = 0;
	}
	
	for( int s = 0; s < child_processes ; s++){
		pid = fork();
		if(pid == 0){
			for (unsigned int  j = limits[s].start; j <  limits[s].end; j++){
				if(j%7 == 0){ 
					limits[s].m_7++;
				}
				if(j%19 == 0){ 
					limits[s].m_19++;
				}
				if((j%7 == 0)	&& (j%19 == 0)){
					limits[s].m_7_19++;
				}
			}
		}
		else{
			wait(NULL);
			exit(0);
			}
	}
	
	for(int w = 0; w < child_processes; w++){
		total_m_7 += limits[w].m_7;
		total_m_19 += limits[w].m_19;
		total_m_7_19 += limits[w].m_7_19;
	}	
	printf("multiplos de 7 -> %d\n", total_m_7);
	printf("multiplos de 19 -> %d\n", total_m_19);
	printf("multiplos de 7 e 19 -> %d\n", total_m_7_19);
			

}
