#include <limits.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lab5-5.h"

void error_and_die(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(){
	/*
	char *memname = "odd_even";
    int i;
		
	shm_unlink(memname);
	int fd = shm_open(memname, O_CREAT | O_RDWR, 0666);
	if (fd == -1)
		error_and_die("shm_open");

	int r = ftruncate(fd, sizeof(shm_region));
	if (r != 0)
		error_and_die("ftruncate");
		
	shm_region *ptr = mmap(0, sizeof(shm_region), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == MAP_FAILED)
		error_and_die("mmap");
	close(fd);
	*/
	
	int fd;
	mkfifo("lab5", 0666);
	fd=open("lab5", O_WRONLY);
	

	int token;
	for(int i=0; i < 100; i++){
	  int rand = random();
	  //ptr->value = rand;
	  write(fd,&rand,sizeof(rand));
	  printf("%d\n", rand);
	  sleep(1);	
      	}

	//shm_unlink(memname);

}
