       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(){
	int fd = open("fifo_teste", O_WRONLY);
	int fd2 = open("fifo_teste", O_RDONLY);
	if(fd == -1){
		mkfifo("fifo_teste", 0666);
fd = open("fifo_teste", O_WRONLY);
	}
	printf("fifo opened\n");
	
	int n= 0;
	while(1){
		char line[100];
		sprintf(line, "%d\n", n);
		write(fd, line, strlen(line)+1);
		n++;
	}
	exit(0);
}
	
