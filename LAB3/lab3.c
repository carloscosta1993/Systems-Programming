#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int main(){
	printf("Parent %d\n",getpid());
	int randomnumber;
	int total_sleep_time = 0, status = 0;
	pid_t pid, wpid;
	for(int i=0; i<10; i++){
		pid = fork();
		srand(getpid());
		randomnumber = rand() % 10;
		if(pid==0){
			sleep(randomnumber);
			printf("IM CHILDREN %d. Time sleep: %d\n", i, randomnumber);
			total_sleep_time = total_sleep_time + randomnumber;
			exit(0);
			}else{total_sleep_time = total_sleep_time + randomnumber;}
		}
	
	while ((wpid = wait(&status)) > 0);
	printf("Total sleep time: %d\n", total_sleep_time);
	exit(0);
}
