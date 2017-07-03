#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>      
#include <sys/mman.h>

int main(){
    int *shm;
    char * filename = "int_file.txt";
    int fd;
    fd = open(filename,  O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    ftruncate(fd, sizeof(int));
    shm = mmap(0, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if (fd == -1)
    {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    
	if (shm == MAP_FAILED)
    {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }


	if(fork()!=0){
		if(fork()!=0){

			int i = 1;
			while(1){
				*shm = i++;
			}
		}else{
			while(1){
				printf("%d \n", *shm);
				sleep(1);
			}
		}		
	}else{
		while(1){
			printf("%d \n", *shm);
			sleep(1);
		}
	}
	
	 if (munmap(shm, sizeof(int)) == -1)
    {
        close(fd);
        perror("Error un-mmapping the file");
        exit(EXIT_FAILURE);
    }
    
    close(fd);
    
    exit(0);
}
